/*
Copyright © 2022-2023  TokiNoBug
This file is part of FractalUtils.

FractalUtils is free software: you can redistribute it and/or modify
                                                                    it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

                                        FractalUtils is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with FractalUtils.  If not, see <https://www.gnu.org/licenses/>.

   Contact with me:
   github:https://github.com/ToKiNoBug
*/

#ifndef FRACTALUTILS_MULTIPRECISIONUTILS_MPINTS_HPP
#define FRACTALUTILS_MULTIPRECISIONUTILS_MPINTS_HPP

#include <boost/multiprecision/cpp_int.hpp>
#include <type_traits>
#include <cstdint>
#include <cstdlib>

namespace boostmp = boost::multiprecision;

namespace fractal_utils {

constexpr bool is_valid_precision(int prec) noexcept {
  if (prec <= 0) {
    return false;
  }

  if (prec == 1) {
    return true;
  }

  if (prec % 2 == 1) {
    return false;
  }

  return is_valid_precision(prec >> 1);
}

constexpr int highest_bit(uint64_t value) noexcept {
  if (value == 0) {
    return -1;
  }
  if (value == 1) {
    return 0;
  }

  return 1 + highest_bit(value >> 1);
}

constexpr int max_precision = 1 << 14;

namespace internal {

template <int precision>
struct multiprecision_int_types {
  static_assert(is_valid_precision(precision),
                "Invalid precision of floating point types");
  using uint_t =
      boostmp::number<boostmp::cpp_int_backend<precision * 32, precision * 32,
                                               boostmp::unsigned_magnitude,
                                               boostmp::unchecked, void>>;

  // boostmp::uint512_t _t;
};

template <>
struct multiprecision_int_types<1> {
  using uint_t = uint32_t;
};
template <>
struct multiprecision_int_types<2> {
  using uint_t = uint64_t;
};

#ifdef __GNUC__
template <>
struct multiprecision_int_types<4> {
  using uint_t = __uint128_t;
};
#endif

}  // namespace internal

template <int precision>
using uint_by_precision_t =
    typename internal::multiprecision_int_types<precision>::uint_t;

namespace internal {
template <typename uint_t, int current_precision>
consteval int extract_precision_uint() {
  static_assert(current_precision <= max_precision,
                "Failed to extract the precision, uint_t seems not to be of "
                "any positive integer precision. (precision of float is 1)");

  if constexpr (std::is_same_v<uint_t,
                               uint_by_precision_t<current_precision>>) {
    return current_precision;
  } else {
    return extract_precision_uint<uint_t, current_precision * 2>();
  }
}
}  // namespace internal

template <typename uintX_t>
constexpr int precision_of_uint_v =
    internal::extract_precision_uint<uintX_t, 1>();

template <typename uintX_t>
constexpr int bits_of_uint_v = 32 * precision_of_uint_v<uintX_t>;

namespace internal {
template <int bits>
struct impl_intX {
  static_assert((bits % 32 == 0) && is_valid_precision(bits / 32),
                "Invalid bits");
};

template <>
struct impl_intX<8> {
  using type = int8_t;
};
template <>
struct impl_intX<16> {
  using type = int16_t;
};
template <>
struct impl_intX<32> {
  using type = int32_t;
};
template <>
struct impl_intX<64> {
  using type = int64_t;
};
}  // namespace internal

template <int bits>
using trivial_intX_t = typename internal::impl_intX<bits>::type;

constexpr int required_bits(int bits) noexcept {
  if (bits <= 0) {
    return 0;
  }

  if (is_valid_precision(bits)) {
    return bits;
  }

  return 1 << (highest_bit(bits) + 1);
}

static_assert(required_bits(3) == 4);
static_assert(required_bits(7) == 8);
static_assert(required_bits(63) == 64);
static_assert(required_bits(4) == 4);
static_assert(required_bits(64) == 64);

template <int bits>
using least_intX_t = trivial_intX_t<required_bits(bits)>;

static_assert(std::is_same_v<least_intX_t<7>, int8_t>);
static_assert(std::is_same_v<least_intX_t<8>, int8_t>);
static_assert(std::is_same_v<least_intX_t<63>, int64_t>);
static_assert(std::is_same_v<least_intX_t<64>, int64_t>);

}  // namespace fractal_utils

#endif  // FRACTALUTILS_MULTIPRECISIONUTILS_MPINTS_HPP
