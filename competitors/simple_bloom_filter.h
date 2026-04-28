#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

template <class KeyType>
class SimpleBloomFilter {
 public:
  void Reset(std::size_t expected_keys) {
    std::size_t target_bits = expected_keys * bits_per_key_;
    if (target_bits < min_bits_) target_bits = min_bits_;
    bit_count_ = NextPowerOfTwo(target_bits);
    word_count_ = bit_count_ / word_bits_;
    bits_.assign(word_count_, 0);
  }

  void Insert(KeyType key) {
    if (bits_.empty()) return;
    const uint64_t key64 = static_cast<uint64_t>(key);
    for (std::size_t i = 0; i < salts_.size(); ++i) {
      const std::size_t bit_index = Hash(key64, salts_[i]) & (bit_count_ - 1);
      bits_[bit_index / word_bits_] |= (uint64_t{1} << (bit_index % word_bits_));
    }
  }

  bool Contains(KeyType key) const {
    if (bits_.empty()) return true;
    const uint64_t key64 = static_cast<uint64_t>(key);
    for (std::size_t i = 0; i < salts_.size(); ++i) {
      const std::size_t bit_index = Hash(key64, salts_[i]) & (bit_count_ - 1);
      if ((bits_[bit_index / word_bits_] & (uint64_t{1} << (bit_index % word_bits_))) == 0) {
        return false;
      }
    }
    return true;
  }

 private:
  static constexpr std::size_t bits_per_key_ = 12;
  static constexpr std::size_t min_bits_ = 64;
  static constexpr std::size_t word_bits_ = 64;
  static constexpr std::array<uint64_t, 3> salts_{{
      0x9e3779b97f4a7c15ULL,
      0xbf58476d1ce4e5b9ULL,
      0x94d049bb133111ebULL,
  }};

  static std::size_t NextPowerOfTwo(std::size_t value) {
    if (value <= 1) return 1;
    --value;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
#if SIZE_MAX > UINT32_MAX
    value |= value >> 32;
#endif
    return value + 1;
  }

  static uint64_t Mix(uint64_t value) {
    value += 0x9e3779b97f4a7c15ULL;
    value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31);
  }

  static uint64_t Hash(uint64_t key, uint64_t salt) {
    return Mix(key ^ salt);
  }

  std::vector<uint64_t> bits_;
  std::size_t bit_count_ = 0;
  std::size_t word_count_ = 0;
};