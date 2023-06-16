#ifndef FRACTALUTILS_MULTIPRECISIONUTILS_MPFLOATS_HPP
#define FRACTALUTILS_MULTIPRECISIONUTILS_MPFLOATS_HPP

// #include <stdfloat>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <type_traits>

#include "mp_ints.hpp"

namespace fractal_utils {

template <typename flt_t>
concept is_boost_multiprecision_float = requires(const flt_t &flt) {
  requires !std::is_trivial_v<flt_t>;
  flt.backend();
  flt.backend().sign();
  flt.backend().exponent();
  flt.backend().bits();
  flt_t::backend_type::bit_count;
  flt_t::backend_type::max_exponent;
};

template <typename flt_t>
concept is_boost_gmp_float = requires(const flt_t &flt) {
  requires !std::is_trivial_v<flt_t>;
  flt.backend();
  flt.backend().data();
  flt.backend().data()->_mp_prec;
  flt.backend().data()->_mp_size;
  flt.backend().data()->_mp_exp;
  *flt.backend().data()->_mp_d;
};

constexpr int suggested_exponent_bits_of(int precision) noexcept {
  if (!is_valid_precision(precision)) {
    return 0;
  }

  if (precision == 1) {
    return 8;
  }
  if (precision == 2) {
    return 11;
  }
  if (precision == 4) {
    return 15;
  }
  if (precision == 8) {
    return 19;
  }

  static_assert(highest_bit(8) == 3);

  const int hb = highest_bit(uint64_t(precision));

  return 19 + 4 * (hb - 3);
}

constexpr int suggested_mantissa_bits_of(int precision) noexcept {
  const int bits = precision * 32;
  return bits - suggested_exponent_bits_of(precision);
}

namespace internal {

template <int precision>
struct multiprecision_float_types {
  static_assert(is_valid_precision(precision),
                "Invalid precision of floating point types");

  static constexpr int exponent_bits = suggested_exponent_bits_of(precision);
  using exponent_t = least_intX_t<exponent_bits>;
  static constexpr exponent_t exponent_min =
      -1 * (exponent_t(1) << (exponent_bits - 1) - 2);
  static constexpr exponent_t exponent_max = exponent_t(1)
                                             << (exponent_bits - 1) - 1;

  using float_t = boostmp::number<
      boostmp::cpp_bin_float<suggested_mantissa_bits_of(precision),
                             boostmp::digit_base_2, void, exponent_t,
                             exponent_min, exponent_max>,
      boostmp::et_off>;
};

template <>
struct multiprecision_float_types<1> {
  using float_t = float;
};
template <>
struct multiprecision_float_types<2> {
  using float_t = double;
};

template <>
struct multiprecision_float_types<4> {
#ifdef __GNUC__
  using float_t = __float128;
#else
  using float_t = boost::multiprecision::cpp_bin_float_quad;
#endif
};

template <>
struct multiprecision_float_types<8> {
  using float_t = boost::multiprecision::cpp_bin_float_oct;
};

}  // namespace internal

template <int precision>
using float_by_precision_t =
    internal::multiprecision_float_types<precision>::float_t;

namespace internal {
template <typename float_t, int current_precision>
consteval int extract_precision_float() {
  static_assert(current_precision <= max_precision,
                "Failed to extract the precision, float_t seems not to be of "
                "any positive integer precision. (precision of float is 1)");

  if constexpr (std::is_same_v<float_t,
                               float_by_precision_t<current_precision>>) {
    return current_precision;
  } else {
    return extract_precision_float<float_t, current_precision * 2>();
  }
}

}  // namespace internal

template <typename float_t>
constexpr int precision_of_float_v =
    internal::extract_precision_float<float_t, 1>();
}  // namespace fractal_utils

#endif  // FRACTALUTILS_MULTIPRECISIONUTILS_MPFLOATS_HPP
