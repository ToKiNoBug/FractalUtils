//
// Created by joseph on 6/15/23.
//

#ifndef FRACTALUTILS_ENCODE_H
#define FRACTALUTILS_ENCODE_H

#include "mp_ints.hpp"
#include "mp_floats.hpp"
#include <span>
#include <optional>

namespace fractal_utils {
namespace internal {
template <typename uintX_t>
void encode_uintX(const uintX_t &bin, std::span<uint8_t> u8_dst,
                  bool is_little_endian = true) noexcept {
  constexpr int bits = bits_of_uint_v<uintX_t>;
  static_assert(bits > 0);
  static_assert(bits % 8 == 0);
  constexpr int bytes = bits / 8;

  assert(u8_dst.size() == bytes);

  for (int byteid = 0; byteid < bytes; byteid++) {
    const int offset = byteid * 8;
    uint8_t val = uint8_t((bin >> offset) & uintX_t(0xFF));
    if (is_little_endian) {
      u8_dst[byteid] = val;
    } else {
      u8_dst[bytes - byteid - 1] = val;
    }
  }
}

template <typename flt_t>
  requires is_boost_multiprecision_float<flt_t>
void encode_boost_floatX(const flt_t &flt, std::span<uint8_t> dst) noexcept {
  static_assert(!std::is_trivial_v<flt_t>);

  constexpr int precision = precision_of_float_v<flt_t>;
  using uintX_t = uint_by_precision_t<precision>;

  constexpr int total_bits = bits_of_uint_v<uintX_t>;

  constexpr int bits = flt_t::backend_type::bit_count;
  constexpr int bits_encoded = bits - 1;
  constexpr int exp_bits = total_bits - bits_encoded - 1;
  constexpr int64_t exp_bias = flt_t::backend_type::max_exponent;

  uintX_t bin{0};

  if (flt.sign() < 0) {
    bin |= 1;
  }
  bin = bin << exp_bits;

  bin |= (flt.backend().exponent() + exp_bias);
  bin = bin << bits_encoded;

  const uintX_t mask = (uintX_t(1) << bits_encoded) - 1;
  bin |= (uintX_t(flt.backend().bits()) & mask);

  encode_uintX<uintX_t>(bin, dst);
}

}  // namespace internal

template <typename float_t>
std::optional<size_t> encode_float(const float_t &flt,
                                   std::span<uint8_t> dest) noexcept {
  constexpr size_t required_bytes = precision_of_float_v<float_t> * 4;
  if (dest.size() < required_bytes) {
    return std::nullopt;
  }
  constexpr bool is_trivial = std::is_trivial_v<float_t>;
  constexpr bool is_boost = is_boost_multiprecision_float<float_t>;

  static_assert(is_trivial || is_boost, "Uknown floating-point stype");

  if constexpr (is_trivial) {
    *reinterpret_cast<float_t *>(dest.data()) = flt;
    return required_bytes;
  }

  if constexpr (is_boost) {
    internal::encode_boost_floatX<float_t>(flt, dest);
    return required_bytes;
  }
  return std::nullopt;
}
}  // namespace fractal_utils

#endif  // FRACTALUTILS_ENCODE_H
