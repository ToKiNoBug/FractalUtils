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

#include "gmp_support.h"

namespace fu_internal = fractal_utils::internal;

size_t fu_internal::limb_num(const __mpf_struct *number) noexcept {
  return number->_mp_prec + 1;
}

size_t fu_internal::required_bytes(const __mpf_struct *number) noexcept {
  const size_t fixed_space = sizeof(number->_mp_prec) +
                             sizeof(number->_mp_size) + sizeof(number->_mp_exp);

  return fixed_space + limb_num(number) * sizeof(mp_limb_t);
}

size_t fu_internal::encode_gmp_float_impl(
    const __mpf_struct *number, std::span<uint8_t> dst_span) noexcept {
  assert(dst_span.size() >= required_bytes(number));
  // const int limb_num = std::abs(number->_mp_size);

  uint8_t *write = dst_span.data();
  write_to_data(&write, number->_mp_prec);
  write_to_data(&write, number->_mp_size);
  write_to_data(&write, number->_mp_exp);

  const size_t limb_bytes = limb_num(number) * sizeof(mp_limb_t);
  memcpy(write, number->_mp_d, limb_bytes);
  write += limb_bytes;

  assert(write <= &*dst_span.end());
  return required_bytes(number);
}

bool fu_internal::decode_gmp_float_impl(std::span<const uint8_t> src,
                                        __mpf_struct *number) noexcept {
  // mpf_clear(number);

  decltype(number->_mp_prec) prec;
  decltype(number->_mp_size) size;
  decltype(number->_mp_exp) exp;

  const uint8_t *srcp = src.data();
  if (src.size() <= sizeof(prec) + sizeof(size) + sizeof(exp)) {
    return false;
  }

  read_from_data(&srcp, prec);
  read_from_data(&srcp, size);
  read_from_data(&srcp, exp);

  size_t limb_n;

  int prec_from_mpf{-1};
  {
    __mpf_struct temp{prec, size, exp, nullptr};
    limb_n = limb_num(&temp);

    prec_from_mpf = mpf_get_prec(&temp);
  }

  const size_t limb_bytes = limb_n * sizeof(mp_limb_t);

  if (&*src.end() - srcp != limb_bytes) {
    return false;
  }

  mpf_set_prec(number, prec_from_mpf);

  assert(number->_mp_prec == prec);
  // assert(std::abs(number->_mp_size) == std::abs(size));
  number->_mp_size = size;
  number->_mp_exp = exp;
  memcpy(number->_mp_d, srcp, limb_bytes);
  return true;
}

size_t fractal_utils::encode_gmp_float(const gmp_float_t &number,
                                       std::span<uint8_t> dest) noexcept {
  const size_t required_bytes =
      fu_internal::required_bytes(number.backend().data());
  if (dest.size() < required_bytes) {
    return 0;
  }

  return internal::encode_gmp_float_impl(number.backend().data(), dest);
}

size_t fractal_utils::encode_gmp_float(const mpf_class &number,
                                       std::span<uint8_t> dest) noexcept {
  const size_t required_bytes = fu_internal::required_bytes(number.get_mpf_t());
  if (dest.size() < required_bytes) {
    return 0;
  }

  return internal::encode_gmp_float_impl(number.get_mpf_t(), dest);
}

std::vector<uint8_t> fractal_utils::encode_gmp_float(
    const gmp_float_t &number) noexcept {
  std::vector<uint8_t> ret;
  ret.resize(internal::required_bytes(number.backend().data()));

  assert(ret.size() == encode_gmp_float(number, ret));
  return ret;
}

std::optional<fractal_utils::gmp_float_t> fractal_utils::decode_gmp_float(
    std::span<const uint8_t> src) noexcept {
  fractal_utils::gmp_float_t ret;

  const bool ok = internal::decode_gmp_float_impl(src, ret.backend().data());
  if (ok) {
    return ret;
  }
  return std::nullopt;
}

std::optional<mpf_class> fractal_utils::decode_gmpxx_float(
    std::span<const uint8_t> src) noexcept {
  mpf_class ret;

  const bool ok = internal::decode_gmp_float_impl(src, ret.get_mpf_t());
  if (ok) {
    return ret;
  }
  return std::nullopt;
}

using fractal_utils::gmp_complex_wrapper;

gmp_complex_wrapper &gmp_complex_wrapper::operator=(
    const gmp_complex_wrapper &src) & noexcept {
  if (&src == this) {
    return *this;
  }
  this->_real = src._real;
  this->_imag = src._imag;
  return *this;
}
gmp_complex_wrapper &gmp_complex_wrapper::operator=(
    gmp_complex_wrapper &&src) & noexcept {
  if (&src == this) {
    return *this;
  }
  this->_real = std::move(src._real);
  this->_imag = std::move(src._imag);
  return *this;
}

gmp_complex_wrapper &gmp_complex_wrapper::add(
    const gmp_complex_wrapper &Z) & noexcept {
  assert(&Z != this);
  this->_real += Z._real;
  this->_imag += Z._imag;
  return *this;
}

void gmp_complex_wrapper::add(const gmp_complex_wrapper &Z,
                              gmp_complex_wrapper &dst) const noexcept {
  assert(this != &Z);
  if (this == &dst) {
    dst.add(Z);
    return;
  }
  assert(&dst != this);
  dst._real = this->_real + Z._real;
  dst._imag = this->_imag + Z._imag;
}

gmp_complex_wrapper &gmp_complex_wrapper::subtract(
    const gmp_complex_wrapper &Z) & noexcept {
  assert(&Z != this);
  this->_real -= Z._real;
  this->_imag -= Z._imag;
  return *this;
}

// dst = a -z
void gmp_complex_wrapper::subtract(const gmp_complex_wrapper &Z,
                                   gmp_complex_wrapper &dst) const noexcept {
  assert(&Z != this);
  if (this == &dst) {
    // a <- a-z
    dst.subtract(Z);
    return;
  }
  dst._real = this->_real - Z._real;
  dst._imag = this->_imag - Z._imag;
}

gmp_complex_wrapper &gmp_complex_wrapper::mult(
    const gmp_complex_wrapper &Z, gmp_complex_buffer &buf) & noexcept {
  assert(&Z != this);
  mpf_class &a = this->_real;
  mpf_class &b = this->_imag;
  const mpf_class &c = Z._real;
  const mpf_class &d = Z._imag;

  mpf_class &bd = buf.float_arr[0];
  mpf_class &ad = buf.float_arr[1];

  bd = b * d;

  ad = a * d;

  // a->ac, b-> bc
  a *= Z._real;
  b *= Z._real;

  a -= bd;
  b += ad;
  return *this;
}

void gmp_complex_wrapper::mult(const gmp_complex_wrapper &Z,
                               gmp_complex_wrapper &dst,
                               gmp_complex_buffer &buf) const noexcept {
  assert(&Z != this);
  assert(&dst != this);
  const mpf_class &a = this->_real;
  const mpf_class &b = this->_imag;
  const mpf_class &c = Z._real;
  const mpf_class &d = Z._imag;

  dst._real = a * c;
  dst._imag = b * c;

  auto &bd = buf.float_arr[0];
  auto &ad = buf.float_arr[1];

  bd = b * d;
  ad = a * d;

  dst._real -= bd;
  dst._imag += ad;
}

gmp_complex_wrapper &gmp_complex_wrapper::divide(
    const gmp_complex_wrapper &Z, gmp_complex_buffer &buf) & noexcept {
  assert(&Z != this);
  mpf_class &a = this->_real;
  mpf_class &b = this->_imag;
  const mpf_class &c = Z._real;
  const mpf_class &d = Z._imag;

  {
    mpf_class &bd = buf.float_arr[0];
    mpf_class &ad = buf.float_arr[1];

    bd = b * d;

    ad = a * d;

    // a->ac, b-> bc
    a *= c;
    b *= c;

    a += bd;
    b -= ad;
  }
  {
    auto &c2 = buf.float_arr[0];
    auto &d2 = buf.float_arr[1];

    c2 = c * c;
    d2 = d * d;
    c2 += d2;  // c2-> c2+d2

    a /= c2;
    b /= c2;
  }

  return *this;
}

void gmp_complex_wrapper::divide(const gmp_complex_wrapper &Z,
                                 gmp_complex_wrapper &dst,
                                 gmp_complex_buffer &buf) const noexcept {
  assert(&Z != this);
  assert(&dst != this);
  const mpf_class &a = this->_real;
  const mpf_class &b = this->_imag;
  const mpf_class &c = Z._real;
  const mpf_class &d = Z._imag;

  dst._real = a * c;
  dst._imag = b * c;

  {
    auto &bd = buf.float_arr[0];
    auto &ad = buf.float_arr[1];

    bd = b * d;
    ad = a * d;

    dst._real += bd;
    dst._imag -= ad;
  }
  {
    auto &c2 = buf.float_arr[0];
    auto &d2 = buf.float_arr[1];

    c2 = c * c;
    d2 = d * d;
    c2 += d2;  // c2-> c2+d2
    dst._real /= c2;
    dst._imag /= c2;
  }
}

gmp_complex_wrapper &gmp_complex_wrapper::inverse(
    gmp_complex_buffer &buf) & noexcept {
  auto &a = this->_real;
  auto &b = this->_imag;
  auto &a2 = buf.float_arr[0];
  auto &b2 = buf.float_arr[1];

  a2 = a * a;
  b2 = b * b;
  a2 += b2;
  // b *= -1;
  b.get_mpf_t()->_mp_size *= -1;  // b=-b;

  a /= a2;
  b /= a2;
  return *this;
}

void gmp_complex_wrapper::inverse(gmp_complex_wrapper &dst,
                                  gmp_complex_buffer &buf) const noexcept {
  assert(this != &dst);
  const auto &a = this->_real;
  const auto &b = this->_imag;
  auto &a2 = buf.float_arr[0];
  auto &b2 = buf.float_arr[1];

  a2 = a * a;
  b2 = b * b;
  a2 += b2;  // a2 = a*a+b*b

  dst._real = a / a2;
  dst._imag = b / a2;

  dst._imag.get_mpf_t()->_mp_size *= -1;
}

gmp_complex_wrapper &gmp_complex_wrapper::square(
    gmp_complex_buffer &buf) & noexcept {
  auto &a = this->_real;
  auto &b = this->_imag;
  auto &a2 = buf.float_arr[0];
  auto &b2 = buf.float_arr[1];

  a2 = a * a;
  b2 = b * b;

  b *= a;
  b *= 2;  // b <- 2ab

  a = a2 - b2;
  return *this;
}

void gmp_complex_wrapper::square(gmp_complex_wrapper &dst,
                                 gmp_complex_buffer &buf) const noexcept {
  assert(this != &dst);
  const auto &a = this->_real;
  const auto &b = this->_imag;

  {
    dst._real = a * a;
    auto &b2 = buf.float_arr[1];
    b2 = b * b;
    dst._real -= b2;
  }
  {
    dst._imag = a * b;
    dst._imag *= 2;
  }
}

gmp_complex_wrapper &gmp_complex_wrapper::power(
    int64_t n, gmp_complex_buffer &buf) & noexcept {
  if (n == 0) {
    this->_real = 1;
    this->_imag = 0;
    return *this;
  }

  if (n == -1) {
    this->inverse(buf);
    return *this;
  }

  const bool n_positive = n > 0;
  n = std::abs(n);
  if (n == 1) {
    ;
  }
  if (n == 2) {
    this->square(buf);
  }
  if (n > 2) {
    auto &this_archive = buf.complex_arr[0];
    this_archive = *this;
    for (int64_t i = 1; i < n; i++) {
      this->mult(this_archive, buf);
    }
  }

  if (!n_positive) {
    this->inverse(buf);
  }
  return *this;
}

void gmp_complex_wrapper::power(int64_t n, gmp_complex_wrapper &dst,
                                gmp_complex_buffer &buf) const noexcept {
  assert(this != &dst);
  if (n == 0) {
    dst._real = 1;
    dst._imag = 0;
    return;
  }

  if (n == -1) {
    this->inverse(dst, buf);
    return;
  }

  const bool n_positive = n > 0;
  n = std::abs(n);
  if (n == 1) {
    dst = *this;
  }
  if (n == 2) {
    this->square(dst, buf);
  }

  if (n > 2) {
    this->square(dst, buf);
    for (int64_t i = 2; i < n; i++) {
      dst.mult(*this, buf);
    }
  }

  if (!n_positive) {
    dst.inverse(buf);
  }
}

std::ostream &fractal_utils::operator<<(std::ostream &os,
                                        const gmp_complex_wrapper &z) noexcept {
  os << z.real().get_d();
  if (z.imag().get_d() >= 0) {
    os << '+';
  }
  os << z.imag().get_d() << 'i';
  return os;
}