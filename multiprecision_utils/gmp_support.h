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
#include <ostream>

#include "mp_ints.hpp"
#include "mp_floats.hpp"
#include "mp_complex.hpp"

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
size_t encode_gmp_float(const mpf_class &number,
                        std::span<uint8_t> dest) noexcept;

std::optional<gmp_float_t> decode_gmp_float(
    std::span<const uint8_t> src) noexcept;
std::optional<mpf_class> decode_gmpxx_float(
    std::span<const uint8_t> src) noexcept;

struct gmp_complex_buffer;
class gmp_complex_wrapper {
 protected:
  mpf_class _real;
  mpf_class _imag;

 public:
  using value_type = mpf_class;
  using buffer_t = gmp_complex_buffer;

  gmp_complex_wrapper() = default;
  gmp_complex_wrapper(const gmp_complex_wrapper &) = default;
  gmp_complex_wrapper(gmp_complex_wrapper &&) = default;
  gmp_complex_wrapper(const mpf_class &r, const mpf_class &i)
      : _real{r}, _imag{i} {}
  gmp_complex_wrapper(mpf_class &&r, mpf_class &&i) : _real{r}, _imag{i} {}

  explicit gmp_complex_wrapper(mp_bitcnt_t precision)
      : _real{0, precision}, _imag{0, precision} {}

  template <typename r_t, typename i_t>
  gmp_complex_wrapper(const r_t &r, const i_t &i, int precision)
      : _real{r, precision}, _imag{i, precision} {}

  auto &real() noexcept { return this->_real; }
  const auto &real() const noexcept { return this->_real; }
  auto &imag() noexcept { return this->_imag; }
  const auto &imag() const noexcept { return this->_imag; }

  gmp_complex_wrapper &operator=(const gmp_complex_wrapper &src) & noexcept;
  gmp_complex_wrapper &operator=(gmp_complex_wrapper &&src) & noexcept;

  gmp_complex_wrapper &add(const gmp_complex_wrapper &Z) & noexcept;
  void add(const gmp_complex_wrapper &Z,
           gmp_complex_wrapper &dst) const noexcept;

  gmp_complex_wrapper &subtract(const gmp_complex_wrapper &Z) & noexcept;
  void subtract(const gmp_complex_wrapper &Z,
                gmp_complex_wrapper &dst) const noexcept;

  gmp_complex_wrapper &mult(const gmp_complex_wrapper &Z,
                            gmp_complex_buffer &buf) & noexcept;
  void mult(const gmp_complex_wrapper &Z, gmp_complex_wrapper &dst,
            gmp_complex_buffer &buf) const noexcept;

  gmp_complex_wrapper &divide(const gmp_complex_wrapper &Z,
                              gmp_complex_buffer &buf) & noexcept;
  void divide(const gmp_complex_wrapper &Z, gmp_complex_wrapper &dst,
              gmp_complex_buffer &buf) const noexcept;

  gmp_complex_wrapper &inverse(gmp_complex_buffer &buf) & noexcept;
  void inverse(gmp_complex_wrapper &dst,
               gmp_complex_buffer &buf) const noexcept;

  gmp_complex_wrapper &square(gmp_complex_buffer &buf) & noexcept;
  void square(gmp_complex_wrapper &dst, gmp_complex_buffer &buf) const noexcept;

  gmp_complex_wrapper &power(int64_t n, gmp_complex_buffer &buf) & noexcept;
  void power(int64_t n, gmp_complex_wrapper &dst,
             gmp_complex_buffer &buf) const noexcept;

  template <typename float_t>
  void to_std_complex(std::complex<float_t> &ret) const noexcept {
    ret.real(this->_real.get_d());
    ret.imag(this->_imag.get_d());
  }

  template <typename float_t>
  std::complex<float_t> to_std_complex() const noexcept {
    static_assert(!std::is_same_v<float_t, mpf_class>);
    std::complex<float_t> ret;
    this->to_std_complex(ret);
    return ret;
  }

  void to_std_complex(std::complex<mpf_class> &ret) const noexcept {
    ret.real() = ret.real();
    ret.imag() = ret.imag();
  }
  std::complex<mpf_class> to_std_complex() const noexcept {
    std::complex<mpf_class> ret;
    this->to_std_complex(ret);
    return ret;
  }
};

std::ostream &operator<<(std::ostream &os,
                         const gmp_complex_wrapper &z) noexcept;

struct gmp_complex_buffer {
 public:
  std::array<mpf_class, 2> float_arr;
  std::array<gmp_complex_wrapper, 1> complex_arr;

 public:
  gmp_complex_buffer() = default;
  explicit gmp_complex_buffer(mp_bitcnt_t prec) {
    for (auto &val : this->float_arr) {
      val.set_prec(prec);
    }
    for (auto &val : this->complex_arr) {
      val.real().set_prec(prec);
      val.imag().set_prec(prec);
    }
  }
};

}  // namespace fractal_utils
#endif  // FRACTALUTILS_GMP_SUPPORT_H
