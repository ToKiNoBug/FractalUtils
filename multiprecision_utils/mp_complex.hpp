//
// Created by joseph on 6/17/23.
//

#ifndef FRACTALUTILS_MP_COMPLEX_HPP
#define FRACTALUTILS_MP_COMPLEX_HPP

#include <complex>
#include <concepts>
#include <boost/multiprecision/complex_adaptor.hpp>
#include <boost/multiprecision/cpp_complex.hpp>
#include "mp_floats.hpp"

namespace fractal_utils {

template <typename T>
concept is_complex = requires(T t, typename T::value_type d) {
  requires std::is_copy_assignable_v<T>;
  requires std::is_move_assignable_v<T>;
  requires std::is_copy_constructible_v<T>;
  requires std::is_move_constructible_v<T>;
  requires !std::assignable_from<typename T::value_type, T>;
  requires !is_boost_multiprecision_float<T>;
  t.real();
  t.imag();
  t + d;
  t += d;
  t - d;
  t -= d;
  t* d;
  t *= d;
  t / d;
  t /= d;
};

static_assert(!is_complex<float>);
static_assert(!is_complex<double>);
static_assert(is_complex<std::complex<float>>);
static_assert(is_complex<std::complex<double>>);

static_assert(!is_complex<boostmp::cpp_bin_float_oct>);
static_assert(is_complex<boostmp::cpp_complex_oct>);

/*
template <typename T>
class fu_complex {
 protected:
  T _real;
  T _imag;

 public:
  using value_type = T;
  fu_complex() = default;
  fu_complex(const fu_complex&) = default;
  fu_complex(fu_complex&&) = default;

  fu_complex(const T& real, const T& imag) : _real{real}, _imag{imag} {}
  fu_complex(T&& real, T&& imag) : _real{real}, _imag{imag} {}

  template <typename A, typename B>
  fu_complex(const A&& a, const B&& b) : _real{a}, _imag{b} {}

  template <typename A>
  fu_complex(const std::complex<A>& src)
      : _real{T{src.real()}}, _imag{T{src.imag()}} {}

  ~fu_complex() = default;

  T& real() noexcept { return this->_real; }
  const T& real() const noexcept { return this->_real; }
  T& imag() noexcept { return this->_imag; }
  const T& imag() const noexcept { return this->_imag; }

  auto& operator=(const fu_complex& src) & noexcept {
    this->_real = src._real;
    this->_imag = src._imag;
    return *this;
  }
  auto& operator=(fu_complex&& src) & noexcept {
    this->_real = std::move(src._real);
    this->_imag = std::move(src._imag);
    return *this;
  }

  template <typename another_t>
  bool operator==(const another_t& another) const noexcept {
    return (this->real() == another.real()) && (this->imag() == another.imag());
  }

 protected:
  template <typename A>
  void add(const A& a) & noexcept {
    if constexpr (is_complex<A>) {
      this->_real += a.real();
      this->_imag += a.imag();
    } else {
      this->_real += a;
    }
  }

 public:
  template <typename A>
  fu_complex operator+(const A& a) const noexcept {
    fu_complex ret{*this};
    ret.add(a);
    return ret;
  }
  template <typename A>
  fu_complex& operator+=(const A& a) & noexcept {
    this->add(a);
    return *this;
  }

 protected:
  template <typename A>
  void subtract(const A& a) & noexcept {
    if constexpr (is_complex<A>) {
      this->_real -= a.real();
      this->_imag -= a.imag();
    } else {
      this->_real -= a;
    }
  }

 public:
  template <typename A>
  fu_complex operator-(const A& a) const noexcept {
    fu_complex ret{*this};
    ret.subtract(a);
    return ret;
  }
  template <typename A>
  fu_complex& operator-=(const A& a) & noexcept {
    this->subtract(a);
    return *this;
  }

 protected:
  template <typename A>
  void times(const A& a) & noexcept {
    if constexpr (is_complex<A>) {
      fu_complex temp_a{this->_real};
      fu_complex temp_b{this->_imag};
      this->_real *= a.real();
      this->_imag *= a.real();

    } else {
      this->_real *= a;
      this->_imag *= a;
    }
  }

 public:
  // static_assert(is_complex<fu_complex<T>>);
};

*/
namespace internal {

template <typename float_t>
auto impl_complex_type_of() {
  if constexpr (std::is_trivial_v<float_t>) {
    return std::complex<float_t>{};
  }
  if constexpr (is_boost_multiprecision_float<float_t>) {
    return boostmp::number<
        boostmp::complex_adaptor<typename float_t::backend_type>>{};
  }

  /*
  if constexpr (is_boost_gmp_float<float_t>) {
    return boostmp::number<
        boostmp::complex_adaptor<typename float_t::backend_type>>{};
  }
  */
}

}  // namespace internal

template <typename float_t>
using complex_type_of = decltype(internal::impl_complex_type_of<float_t>());

static_assert(std::is_same_v<complex_type_of<float>, std::complex<float>>);
static_assert(std::is_same_v<complex_type_of<double>, std::complex<double>>);
/*
template <typename T>
class complex_wrapper {
 protected:
  T _real;
  T _imag;

 public:
  using value_type = T;
  complex_wrapper() = default;
  complex_wrapper(const complex_wrapper&) = default;
  complex_wrapper(complex_wrapper&&) = default;

  struct buffer_t {
    std::array<T, 2> array;
  };

  static buffer_t& buffer() noexcept {
    thread_local buffer_t buf;
    return buf;
  }

  T& real() noexcept { return this->_real; }
  const T& real() const noexcept { return this->_real; }
  T& imag() noexcept { return this->_imag; }
  const T& imag() const noexcept { return this->_imag; }

  auto& operator=(const complex_wrapper& src) & noexcept {
    this->_real = src._real;
    this->_imag = src._imag;
    return *this;
  }
  auto& operator=(complex_wrapper&& src) & noexcept {
    this->_real = std::move(src._real);
    this->_imag = std::move(src._imag);
    return *this;
  }

  auto& operator+=(const complex_wrapper& B) & noexcept {
    this->_real += B._real;
    this->_imag += B._imag;
    return *this;
  }

  auto& operator-=(const complex_wrapper& B) & noexcept {
    this->_real -= B._real;
    this->_imag -= B._imag;
    return *this;
  }

  auto& operator*=(const complex_wrapper& Z) & noexcept {
    //(a+bi)*(c+di) = (ac-bd) + (bc+ad)i
    // a->ac, b-> bc
    // var1=bd, var2=ad
    // a-=var1, b+=var2

    T& a = this->_real;
    T& b = this->_imag;
    const T& c = Z._real;
    const T& d = Z._imag;

    auto& buf = complex_wrapper::buffer();
    T& bd = buf.array[0];
    T& ad = buf.array[1];

    bd = d;
    bd *= b;

    ad = d;
    ad *= a;

    // a->ac, b-> bc
    a *= Z._real;
    b *= Z._real;

    a -= bd;
    b += ad;
    return *this;
  }
};
 */

/**
 * A complex number with buffer to avoid calling constructor during computation,
 * usually designed for multiprecision types with memory allocation.
 * tparam T
 * tparam cplx_t

template <typename T, typename cplx_t = std::complex<T>>
class complex_wrapper {
 public:
  struct buffer_t {
    cplx_t buf_cplx_0;
    cplx_t buf_cplx_1;
    T buf_t_0;
    T buf_t_1;
  };

 protected:
  cplx_t _value;
  buffer_t _buffer;

 public:
  complex_wrapper() = default;
  complex_wrapper(const complex_wrapper&) = default;
  complex_wrapper(complex_wrapper&&) = default;

  auto& unwrap() noexcept { return this->_value; }
  const auto& unwrap() const noexcept { return this->_value; }

  auto& buffer() noexcept { return this->_buffer; }
  const auto& buffer() const noexcept { return this->_buffer; }

  template <typename A>
  complex_wrapper& operator+=(const A& a) noexcept {
    this->_value += a;
    return *this;
  }
  template <typename A>
  complex_wrapper& operator-=(const A& a) noexcept {
    this->_value -= a;
    return *this;
  }

  template <typename A>
  complex_wrapper& operator*=(const A& a) noexcept {
    this->_buffer.buf_t_0 = this->_value.imag();  // buf_t_0=b
    this->_buffer.buf_t_0 *= a.imag();            // buf_t_0=bd

    this->_value.real() *=
#warning here
  }
};*/

}  // namespace fractal_utils

#endif  // FRACTALUTILS_MP_COMPLEX_HPP
