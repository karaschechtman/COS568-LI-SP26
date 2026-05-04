#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <fstream>
#include <limits>
#include "../util.h"
#include "base.h"
#include "dtl_bloom_filter_wrapper.h"
#include "./lipp/src/core/lipp.h"
#include "pgm_index_dynamic.hpp"


template <class KeyType, class SearchClass, size_t pgm_error = 16, uint32_t flush_ratio = 50>
class HybridPGMLIPP : public Base<KeyType> {
 public:
  HybridPGMLIPP(const std::vector<int>& params) {}

  ~HybridPGMLIPP() = default;

  uint64_t Build(const std::vector<KeyValue<KeyType>>& data, size_t num_threads) {

    total_keys_ = data.size();

    bloom_filter_.Reset(data.size());
    for (const auto& itm : data) {
      bloom_filter_.Insert(itm.key);
    }

    std::vector<std::pair<KeyType, uint64_t>> bulk;
    bulk.reserve(data.size());
    for (const auto& itm : data) {
      bulk.emplace_back(itm.key, itm.value);
    }

    return util::timing([&] {
      lipp_.bulk_load(bulk.data(), bulk.size());
    });
  }

  void Insert(const KeyValue<KeyType>& data, uint32_t thread_id) {
    bloom_filter_.Insert(data.key);

    dpgm_.insert(data.key, data.value);
    dpgm_size_++;

    size_t threshold = (total_keys_ * flush_ratio) / 100;

    // Flush when DPGM reaches the configured ratio of the initial dataset.
    if (dpgm_size_ >= threshold) {
      FlushToLIPP();
    }
  }

  size_t EqualityLookup(const KeyType& key, uint32_t thread_id) const {
    if (!bloom_filter_.Contains(key)) return util::NOT_FOUND;

    uint64_t value;

    // LIPP first
    if (lipp_.find(key, value)) {
      return value;
    }

    // DPGM fallback
    auto it = dpgm_.find(key);
    if (it != dpgm_.end()) {
      return it->value();
    }

    return util::NOT_FOUND;
  }

  uint64_t RangeQuery(const KeyType& lower, const KeyType& upper, uint32_t thread_id) const {
    uint64_t result = 0;

    // DPGM scan
    auto it = dpgm_.lower_bound(lower);
    while (it != dpgm_.end() && it->key() <= upper) {
      result += it->value();
      ++it;
    }

    // LIPP scan
    auto lit = lipp_.lower_bound(lower);
    while (lit != lipp_.end() && lit->comp.data.key <= upper) {
      result += lit->comp.data.value;
      ++lit;
    }

    return result;
  }

  std::string name() const {
    return "HybridPGMLIPP";
  }

  std::size_t size() const {
    return dpgm_.size_in_bytes() + lipp_.index_size();
  }

  std::vector<std::string> variants() const {
    return {
      SearchClass::name(),
      std::to_string(pgm_error),
      std::to_string(flush_ratio)
    };
  }

 private:
  void FlushToLIPP() {
    auto it = dpgm_.lower_bound(std::numeric_limits<KeyType>::min());

    while (it != dpgm_.end()) {
      lipp_.insert(it->key(), it->value());
      ++it;
    }

    dpgm_ = DynamicPGMIndex<KeyType, uint64_t, SearchClass,
                        PGMIndex<KeyType, SearchClass, pgm_error, 16>>();
    dpgm_size_ = 0;
  }

 private:
  DynamicPGMIndex<KeyType, uint64_t, SearchClass,
      PGMIndex<KeyType, SearchClass, pgm_error, 16>> dpgm_;

  LIPP<KeyType, uint64_t> lipp_;
  DTLBloomFilterWrapper<KeyType> bloom_filter_;

  size_t dpgm_size_ = 0;
  size_t total_keys_ = 0;
};