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

#ifndef FRACTALUTILS_DECODE_HPP
#define FRACTALUTILS_DECODE_HPP

#include "mp_ints.hpp"
#include "mp_floats.hpp"
#include <span>

#ifdef FRACTALUTILS_MULTIPRECISIONUTILS_GMP_SUPPORT
#include "gmp_support.h"
#endif

#ifdef FRACTALUTILS_MULTIPRECISIONUTILS_MPFR_SUPPORT
#include "mpfr_support.h"
#endif

namespace fractal_utils {
namespace internal {

template <typename uintX_t>
uintX_t decode_uintX(std::span<const uint8_t> u8_src,
                     bool is_little_endian = true) noexcept {
  constexpr int bits = bits_of_uint_v<uintX_t>;
  static_assert(bits > 0);
  static_assert(bits % 8 == 0);
  constexpr int bytes = bits / 8;

  assert(u8_src.size() == bytes);

  uintX_t ret{0};

  for (int byteid = 0; byteid < bytes; byteid++) {
    const int offset = byteid * 8;
    uintX_t val{~uintX_t(0)};
    if (is_little_endian) {
      val = u8_src[byteid];
    } else {
      val = u8_src[bytes - byteid - 1];
    }

    val = val << offset;
    ret |= val;
  }

  return ret;
}

template <typename flt_t>
  requires is_boost_multiprecision_float<flt_t>
flt_t decode_boost_cpp_bin_float(std::span<const uint8_t> src) noexcept {
  static_assert(!std::is_trivial_v<flt_t>);

  constexpr int precision = precision_of_float_v<flt_t>;
  assert(src.size() == precision * 4);

  using uintX_t = uint_by_precision_t<precision>;

  constexpr int total_bits = bits_of_uint_v<uintX_t>;

  constexpr int bits = flt_t::backend_type::bit_count;
  constexpr int bits_encoded = bits - 1;
  constexpr int exp_bits = total_bits - bits_encoded - 1;
  constexpr int64_t exp_bias = flt_t::backend_type::max_exponent;

  const uintX_t bin = decode_uintX<uintX_t>(src);

  flt_t result;

  result.backend().sign() = bool(bin >> (total_bits - 1));

  const auto exp_value = (bin >> bits_encoded) & ((uintX_t(1) << exp_bits) - 1);

  using exp_t = std::decay_t<decltype(result.backend().exponent())>;

  result.backend().exponent() = exp_t(exp_value) - exp_bias;

  uintX_t mantissa = bin & ((uintX_t(1) << bits_encoded) - 1);
  if (exp_value != 0) {
    mantissa |= (uintX_t(1) << bits_encoded);
  }

  {
    using uint_bits_t =
        boost::multiprecision::number<boost::multiprecision::cpp_int_backend<
            bits, bits, boost::multiprecision::unsigned_magnitude,
            boost::multiprecision::unchecked, void>>;

    uint_bits_t temp = mantissa.template convert_to<uint_bits_t>();

    result.backend().bits() = temp.backend();
  }

  return result;
}

template <typename float_t>
std::optional<float_t> decode_float(
    std::span<const uint8_t, precision_of_float_v<float_t> * 4> src) noexcept {}
}  // namespace internal

template <typename float_t>
std::optional<float_t> decode_float(std::span<const uint8_t> src) noexcept {
  constexpr bool is_trivial = std::is_trivial_v<float_t>;
  constexpr bool is_boost = is_boost_multiprecision_float<float_t>;
  constexpr bool is_boost_gmp = is_boost_gmp_float<float_t>;
  constexpr bool is_gmpxx = is_gmpxx_float<float_t>;
  constexpr bool is_boost_mpfr = is_boost_mpfr_float<float_t>;

  constexpr bool is_ieee = is_trivial || is_boost;

  static_assert(
      is_trivial || is_boost || is_boost_gmp || is_gmpxx || is_boost_mpfr,
      "Unknown floating-point types");
  if constexpr (is_ieee) {
    constexpr size_t boost_expected_bytes = precision_of_float_v<float_t> * 4;
    if (src.size() != boost_expected_bytes) {
      return std::nullopt;
    }

    if constexpr (is_trivial) {
      return *reinterpret_cast<const float_t *>(src.data());
    }

    if constexpr (is_boost) {
      return internal::decode_boost_cpp_bin_float<float_t>(src);
    }
  }

#ifndef FRACTALUTILS_MULTIPRECISIONUTILS_GMP_SUPPORT
  static_assert(!(is_boost_gmp || is_gmpxx),
                "GMP support is disabled, there is not rule to decode boost "
                "wrapped gmp types.");
#else
  if constexpr (is_boost_gmp) {
    return decode_gmp_float(src);
  }
  if constexpr (is_gmpxx) {
    return decode_gmpxx_float(src);
  }
#endif

#ifndef FRACTALUTILS_MULTIPRECISIONUTILS_GMP_SUPPORT
  static_assert(!is_boost_mpfr,
                "MPFR support is disabled, there is not rule to decode boost "
                "wrapped mpfr types.");
#else
  if constexpr (is_boost_mpfr) {
    return decode_boost_mpfr_float(src);
  }
#endif

  return std::nullopt;
}

}  // namespace fractal_utils
#endif  // FRACTALUTILS_DECODE_HPP
