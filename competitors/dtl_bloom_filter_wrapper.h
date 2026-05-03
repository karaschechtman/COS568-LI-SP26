#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include <dtl/bloomfilter/bloomfilter_logic.hpp>

// Wrapper around dtl::bloomfilter_logic that provides SimpleBloomFilter-compatible interface.
// This keeps the compile-time footprint much smaller than the blocked bloom filter variants.
template <class KeyType>
class DTLBloomFilterWrapper {
 public:
  void Reset(std::size_t expected_keys) {
    std::size_t target_bits = expected_keys * bits_per_key_;
    if (target_bits < min_bits_) target_bits = min_bits_;

    logic_ = std::make_unique<dtl::bloomfilter_logic<uint32_t>>(target_bits, hash_functions_);
    filter_data_.assign(logic_->size(), 0);
  }

  void Insert(KeyType key) {
    if (!logic_ || filter_data_.empty()) return;
    logic_->insert(filter_data_.data(), static_cast<uint32_t>(key));
  }

  bool Contains(KeyType key) const {
    if (!logic_ || filter_data_.empty()) return true;
    return logic_->contains(filter_data_.data(), static_cast<uint32_t>(key));
  }

 private:
  static constexpr std::size_t bits_per_key_ = 12;
  static constexpr std::size_t min_bits_ = 64;
  static constexpr uint32_t hash_functions_ = 3;

  std::unique_ptr<dtl::bloomfilter_logic<uint32_t>> logic_;
  std::vector<uint32_t> filter_data_;
};
