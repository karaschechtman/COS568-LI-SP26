#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "../util.h"
#include "base.h"
#include "./lipp/src/core/lipp.h"
#include "pgm_index_dynamic.hpp"

template <class KeyType, class SearchClass, size_t pgm_error = 16, uint32_t flush_ratio = 5>
class HybridPGMLIPP : public Base<KeyType> {
 public:
  HybridPGMLIPP(const std::vector<int>& params) { }

  uint64_t Build(const std::vector<KeyValue<KeyType>>& data, size_t num_threads) {
    total_keys_ = data.size();

    // Load all initial data into LIPP
    std::vector<std::pair<KeyType, uint64_t>> loading_data;
    loading_data.reserve(data.size());
    for (const auto& itm : data) {
      loading_data.push_back(std::make_pair(itm.key, itm.value));
    }

    uint64_t build_time = util::timing([&] {
      lipp_.bulk_load(loading_data.data(), loading_data.size());
    });

    return build_time;
  }

  size_t EqualityLookup(const KeyType& lookup_key, uint32_t thread_id) const {
    // Check DPGM first for exact match
    auto it = dpgm_.find(lookup_key);
    if (it != dpgm_.end()) {
      return it->value();
    }

    // Fall back to LIPP
    uint64_t value;
    if (!lipp_.find(lookup_key, value)) {
      return util::NOT_FOUND;
    }
    return value;
  }

  uint64_t RangeQuery(const KeyType& lower_key, const KeyType& upper_key, uint32_t thread_id) const {
    // Range query: collect from both DPGM and LIPP
    uint64_t result = 0;

    // Query DPGM
    auto it_dpgm = dpgm_.lower_bound(lower_key);
    while (it_dpgm != dpgm_.end() && it_dpgm->key() <= upper_key) {
      result += it_dpgm->value();
      ++it_dpgm;
    }

    // Query LIPP
    auto it_lipp = lipp_.lower_bound(lower_key);
    while (it_lipp != lipp_.end() && it_lipp->comp.data.key <= upper_key) {
      result += it_lipp->comp.data.value;
      ++it_lipp;
    }

    return result;
  }

  void Insert(const KeyValue<KeyType>& data, uint32_t thread_id) {
    // Add to DPGM
    dpgm_.insert(data.key, data.value);
    dpgm_element_count_++;
    total_keys_++;

    // Calculate flush threshold based on current total
    size_t current_threshold = (total_keys_ * flush_ratio) / 100;
    if (current_threshold == 0) current_threshold = 1;

    // Check if we need to flush
    if (dpgm_element_count_ >= current_threshold) {
      Flush();
    }
  }

  std::string name() const { return "HybridPGMLIPP"; }

  std::size_t size() const {
    return dpgm_.size_in_bytes() + lipp_.index_size();
  }

  bool applicable(bool unique, bool range_query, bool insert, bool multithread, 
                  const std::string& ops_filename) const {
    std::string name = SearchClass::name();
    // LIPP doesn't support multithread and requires unique keys
    // DPGM doesn't support LinearAVX or multithread
    return name != "LinearAVX" && !multithread && unique;
  }

  std::vector<std::string> variants() const {
    std::vector<std::string> vec;
    vec.push_back(SearchClass::name());
    vec.push_back(std::to_string(pgm_error));
    vec.push_back(std::to_string(flush_ratio) + "%");
    return vec;
  }

 private:
  // Naive flush: extract all from DPGM and insert into LIPP
  void Flush() {

    // Extract all data from DPGM
    auto it = dpgm_.begin();
    while (it != dpgm_.end()) {
      lipp_.insert(it->key(), it->value());
      ++it;
    }

    // Clear DPGM by rebuilding it empty
    dpgm_ = DynamicPGMIndex<KeyType, uint64_t, SearchClass, 
                             PGMIndex<KeyType, SearchClass, pgm_error, 16>>();
    
    // Reset element count
    dpgm_element_count_ = 0;
  }

  DynamicPGMIndex<KeyType, uint64_t, SearchClass, 
                  PGMIndex<KeyType, SearchClass, pgm_error, 16>> dpgm_;
  LIPP<KeyType, uint64_t> lipp_;

  size_t total_keys_ = 0;
  size_t dpgm_element_count_ = 0; 
};
