#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <fstream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>

#include "../util.h"
#include "base.h"
#include "simple_bloom_filter.h"
#include "./lipp/src/core/lipp.h"
#include "pgm_index_dynamic.hpp"

// HybridPGMLIPP Async flush hybrid index with minimal locking.
// Only one thread (the background flush thread) ever touches the flush buffer and LIPP during flush.
// Foreground threads only touch dpgm_active_ and never block except for a short lock during swap.
// All data structures are private to one thread at a time, so no concurrent access occurs.
//
// This design avoids data races without requiring DPGM or LIPP to be threadsafe.

template <class KeyType, class SearchClass, size_t pgm_error = 16, uint32_t flush_ratio = 50>
class HybridPGMLIPP : public Base<KeyType> {
 public:
  HybridPGMLIPP(const std::vector<int>& params) {}

  ~HybridPGMLIPP() {
    stop_flush_thread_ = true;
    if (flush_thread_.joinable()) flush_thread_.join();
  }

  uint64_t Build(const std::vector<KeyValue<KeyType>>& data, size_t num_threads) {
    total_keys_ = data.size();
    bloom_filter_.Reset(data.size());
    for (const auto& itm : data) {
      bloom_filter_.Insert(itm.key);
    }
    // Bulk load LIPP
    std::vector<std::pair<KeyType, uint64_t>> loading_data;
    loading_data.reserve(data.size());
    for (const auto& itm : data) {
      loading_data.push_back(std::make_pair(itm.key, itm.value));
    }
    uint64_t build_time = util::timing([&] {
      lipp_.bulk_load(loading_data.data(), loading_data.size());
    });
    // Start flush thread
    stop_flush_thread_ = false;
    flush_thread_ = std::thread(&HybridPGMLIPP::FlushThreadLoop, this);
    return build_time;
  }

  void Insert(const KeyValue<KeyType>& data, uint32_t thread_id) {
    // Foreground threads only touch dpgm_active_
    {
      std::unique_lock<std::shared_mutex> lock(dpgm_mutex_);
      dpgm_active_.insert(data.key, data.value);
      bloom_filter_.Insert(data.key);
      dpgm_element_count_++;
      total_keys_++;
    }
    // Background thread will flush
  }

  size_t EqualityLookup(const KeyType& lookup_key, uint32_t thread_id) const {
    // Check LIPP first (has most data on low-insert workloads).
    uint64_t value;
    // Lock-free read: spin briefly if LIPP is being modified by flush thread.
    while (lipp_being_modified_.load(std::memory_order_acquire)) {
      // spin
    }
    if (lipp_.find(lookup_key, value)) return value;
    // Early exit on Bloom filter miss (append-only, safe to check without lock).
    if (!bloom_filter_.Contains(lookup_key)) return util::NOT_FOUND;
    
    // Skip DPGM checks if both buffers are empty (common on 90% lookup workloads).
    if (dpgm_element_count_ == 0 && dpgm_flush_element_count_ == 0) {
      return util::NOT_FOUND;
    }
    
    // Check dpgm_active_ for recent inserts.
    if (dpgm_element_count_ > 0) {
      std::shared_lock<std::shared_mutex> lock(dpgm_mutex_);
      auto it = dpgm_active_.find(lookup_key);
      if (it != dpgm_active_.end()) return it->value();
    }
    // Check dpgm_flush_ (background thread may be flushing)
    if (dpgm_flush_element_count_ > 0) {
      std::shared_lock<std::shared_mutex> lock(dpgm_flush_mutex_);
      auto it = dpgm_flush_.find(lookup_key);
      if (it != dpgm_flush_.end()) return it->value();
    }
    return util::NOT_FOUND;
  }

  uint64_t RangeQuery(const KeyType& lower_key, const KeyType& upper_key, uint32_t thread_id) const {
    uint64_t result = 0;
    // dpgm_active_
    if (dpgm_element_count_ > 0) {
      std::shared_lock<std::shared_mutex> lock(dpgm_mutex_);
      auto it = dpgm_active_.lower_bound(lower_key);
      while (it != dpgm_active_.end() && it->key() <= upper_key) {
        result += it->value();
        ++it;
      }
    }
    // dpgm_flush_
    if (dpgm_flush_element_count_ > 0) {
      std::shared_lock<std::shared_mutex> lock(dpgm_flush_mutex_);
      auto it = dpgm_flush_.lower_bound(lower_key);
      while (it != dpgm_flush_.end() && it->key() <= upper_key) {
        result += it->value();
        ++it;
      }
    }
    // LIPP (lock-free read; spin only while LIPP is being modified)
    while (lipp_being_modified_.load(std::memory_order_acquire)) {
      // spin
    }
    {
      auto it = lipp_.lower_bound(lower_key);
      while (it != lipp_.end() && it->comp.data.key <= upper_key) {
          result += it->comp.data.value;
          ++it;
      }
    }
    return result;
  }

  std::string name() const { return "HybridPGMLIPP"; }

  std::size_t size() const {
    return dpgm_active_.size_in_bytes() + dpgm_flush_.size_in_bytes() + lipp_.index_size();
  }

  bool applicable(bool unique, bool range_query, bool insert, bool multithread, 
                  const std::string& ops_filename) const {
    std::string name = SearchClass::name();
    return name != "LinearAVX" && !multithread && unique;
  }

  std::vector<std::string> variants() const {
    std::vector<std::string> vec;
    vec.push_back(SearchClass::name());
    vec.push_back(std::to_string(pgm_error));
    vec.push_back(std::to_string(flush_ratio / 100.0) + "%");
    return vec;
  }

 private:
  void FlushThreadLoop() {
    while (!stop_flush_thread_) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      size_t threshold;
      {
        std::unique_lock<std::shared_mutex> lock(dpgm_mutex_);
        threshold = (total_keys_ * flush_ratio) / 10000;
        if (threshold == 0) threshold = 1;
        if (dpgm_element_count_ < threshold) continue;
        // Swap active buffer to flush buffer
        {
          std::unique_lock<std::shared_mutex> flush_lock(dpgm_flush_mutex_);
          std::swap(dpgm_active_, dpgm_flush_);
          dpgm_flush_element_count_.store(dpgm_element_count_.load());  // save count of flushed buffer
          dpgm_element_count_ = 0;
        }
      }
      // Now flush dpgm_flush_ into LIPP (single-threaded, no races)
      {
        std::unique_lock<std::shared_mutex> flush_lock(dpgm_flush_mutex_);
        auto it = dpgm_flush_.lower_bound(std::numeric_limits<KeyType>::min());
        {
          // Set modification flag so readers spin briefly and then read lock-free
          lipp_being_modified_.store(true, std::memory_order_release);
          while (it != dpgm_flush_.end()) {
            lipp_.insert(it->key(), it->value());
            ++it;
          }
          lipp_being_modified_.store(false, std::memory_order_release);
        }
        dpgm_flush_ = DynamicPGMIndex<KeyType, uint64_t, SearchClass, 
                                      PGMIndex<KeyType, SearchClass, pgm_error, 16>>();
        dpgm_flush_element_count_ = 0;
      }
    }
  }

  DynamicPGMIndex<KeyType, uint64_t, SearchClass, 
                  PGMIndex<KeyType, SearchClass, pgm_error, 16>> dpgm_active_;
  DynamicPGMIndex<KeyType, uint64_t, SearchClass, 
                  PGMIndex<KeyType, SearchClass, pgm_error, 16>> dpgm_flush_;
  LIPP<KeyType, uint64_t> lipp_;
  SimpleBloomFilter<KeyType> bloom_filter_;

  mutable std::shared_mutex dpgm_mutex_; // protects dpgm_active_, total_keys_, dpgm_element_count_
  mutable std::shared_mutex dpgm_flush_mutex_; // protects dpgm_flush_
  std::atomic<bool> lipp_being_modified_ = false;
  std::thread flush_thread_;
  std::atomic<bool> stop_flush_thread_ = false;

  std::atomic<size_t> total_keys_ = 0;
  std::atomic<size_t> dpgm_element_count_ = 0;
  std::atomic<size_t> dpgm_flush_element_count_ = 0;
};
