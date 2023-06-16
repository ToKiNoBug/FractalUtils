//
// Created by joseph on 6/16/23.
//

#ifndef FRACTALUTILS_GMP_SUPPORT_H
#define FRACTALUTILS_GMP_SUPPORT_H

#include <gmp.h>
#include <gmpxx.h>
#include <boost/multiprecision/gmp.hpp>
#include <type_traits>
#include <cstdint>
#include <concepts>
#include <vector>
#include <optional>
#include <span>

#include "mp_ints.hpp"
#include "mp_floats.hpp"

namespace fractal_utils {

namespace internal {

#ifdef __GNUC__
constexpr int max_trivial_precision = 4;
#else
constexpr int max_trivial_precision == 2;
#endif

size_t limb_num(const __mpf_struct *number) noexcept;

size_t required_bytes(const __mpf_struct *number) noexcept;

template <typename T>
  requires std::is_trivial_v<T>
void write_to_data(uint8_t **datapp, T t) noexcept {
  *reinterpret_cast<T *>(*datapp) = t;
  *datapp += sizeof(T);
}

size_t encode_gmp_float_impl(const __mpf_struct *number,
                             std::span<uint8_t> dst_span) noexcept;

template <typename T>
  requires std::is_trivial_v<T>
void read_from_data(const uint8_t **datapp, T &t) noexcept {
  t = *reinterpret_cast<const T *>(*datapp);
  *datapp += sizeof(T);
  // return t;
}

bool decode_gmp_float_impl(std::span<const uint8_t> src,
                           __mpf_struct *number) noexcept;

}  // namespace internal

/*
template <int precision>
using gmp_float_by_precision_t = std::conditional_t<
    precision <= internal::max_trivial_precision,
    float_by_precision_t<precision>,
    boostmp::number<boostmp::gmp_float<suggested_mantissa_bits_of(precision)>>>;

*/

using gmp_float_t = boostmp::number<boostmp::gmp_float<0>>;

inline size_t required_bytes_of(const gmp_float_t &number) noexcept {
  return internal::required_bytes(number.backend().data());
}
size_t encode_gmp_float(const gmp_float_t &number,
                        std::span<uint8_t> dest) noexcept;
std::vector<uint8_t> encode_gmp_float(const gmp_float_t &number) noexcept;

std::optional<gmp_float_t> decode_gmp_float(
    std::span<const uint8_t> src) noexcept;

}  // namespace fractal_utils
#endif  // FRACTALUTILS_GMP_SUPPORT_H
