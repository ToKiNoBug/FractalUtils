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

#include "mpfr_support.h"
#include "gmp_support.h"

namespace fui = fractal_utils::internal;

size_t fui::limb_num(const __mpfr_struct* number) noexcept {
  return ceil(double(number->_mpfr_prec) / mp_bits_per_limb);
}

size_t fui::limb_bytes(const __mpfr_struct* number) noexcept {
  return limb_num(number) * sizeof(mp_limb_t);
}

size_t fui::required_bytes_of(const __mpfr_struct* number) noexcept {
  return fixed_part_bytes_of_mpfr() + limb_bytes(number);
}

size_t fui::encode_mpfr_c_struct(const __mpfr_struct* number,
                                 std::span<uint8_t> dest) noexcept {
  const size_t required_bytes = required_bytes_of(number);
  if (dest.size() < required_bytes) {
    return 0;
  }

  uint8_t* write = dest.data();

  fui::write_to_data(&write, number->_mpfr_prec);
  fui::write_to_data(&write, number->_mpfr_sign);
  fui::write_to_data(&write, number->_mpfr_exp);

  memcpy(write, number->_mpfr_d, limb_bytes(number));
  write += limb_bytes(number);

  assert(write - dest.data() <= dest.size());
  return required_bytes;
}

bool fui::decode_mpfr_c_struct(std::span<const uint8_t> code,
                               __mpfr_struct* number) noexcept {
  if (code.size() < fixed_part_bytes_of_mpfr()) {
    return false;
  }

  __mpfr_struct temp;

  const uint8_t* src = code.data();

  read_from_data(&src, temp._mpfr_prec);
  read_from_data(&src, temp._mpfr_sign);
  read_from_data(&src, temp._mpfr_exp);

  const size_t limb_bytes = fui::limb_bytes(&temp);

  if (code.size() != limb_bytes + fixed_part_bytes_of_mpfr()) {
    return false;
  }

  mpfr_set_prec(number, mpfr_get_prec(&temp));

  assert(number->_mpfr_prec == temp._mpfr_prec);

  number->_mpfr_exp = temp._mpfr_exp;
  number->_mpfr_sign = temp._mpfr_sign;

  memcpy(number->_mpfr_d, src, limb_bytes);

  return true;
}

size_t fractal_utils::encode_boost_mpfr_float(
    const boostmp::mpfr_float& num, std::span<uint8_t> dest) noexcept {
  return internal::encode_mpfr_c_struct(num.backend().data(), dest);
}

std::vector<uint8_t> fractal_utils::encode_boost_mpfr_float(
    const boostmp::mpfr_float& num) noexcept {
  const size_t required_bytes = fui::required_bytes_of(num.backend().data());

  std::vector<uint8_t> ret;
  ret.resize(required_bytes);

  const size_t written_bytes = encode_boost_mpfr_float(num, ret);
  assert(written_bytes == ret.size());

  return ret;
}

std::optional<boostmp::mpfr_float> fractal_utils::decode_boost_mpfr_float(
    const std::span<const uint8_t> code) noexcept {
  boostmp::mpfr_float ret;

  const bool ok = internal::decode_mpfr_c_struct(code, ret.backend().data());

  if (ok) {
    return ret;
  }
  return std::nullopt;
}

void resolution_around(mpfr_srcptr a, mpfr_ptr dst, mpfr_ptr buffer) noexcept;

uint32_t find_min_required_precision(boostmp::mpfr_float a,
                                     boostmp::mpfr_float& resolution,
                                     boostmp::mpfr_float& buf1,
                                     const boostmp::mpfr_float& unit) noexcept;

uint32_t fractal_utils::required_precision_of(const boostmp::mpfr_float& center,
                                              const boostmp::mpfr_float& span,
                                              uint32_t num_pixels) noexcept {
  assert(num_pixels > 0);
  assert(mpfr_number_p(center.backend().data()));
  assert(mpfr_number_p(span.backend().data()));
  assert(span != 0);
  boostmp::mpfr_float unit{0,
                           uint32_t(mpfr_get_prec(span.backend().data()) + 20)};

  unit = span / num_pixels;
  mpfr_abs(unit.backend().data(), unit.backend().data(), MPFR_RNDN);
  assert(unit > 0);

  boostmp::mpfr_float max_abs;
  {
    boostmp::mpfr_float upper = center + span / 2;
    boostmp::mpfr_float lower = center - span / 2;
    mpfr_abs(upper.backend().data(), upper.backend().data(), MPFR_RNDN);
    mpfr_abs(lower.backend().data(), lower.backend().data(), MPFR_RNDN);
    mpfr_max(max_abs.backend().data(), lower.backend().data(),
             lower.backend().data(), MPFR_RNDN);
  }

  boostmp::mpfr_float buf1{0, max_abs.precision()};
  boostmp::mpfr_float resolution{0, max_abs.precision()};

  return find_min_required_precision(max_abs, resolution, buf1, unit);
}

void resolution_around(mpfr_srcptr a, mpfr_ptr dst,
                       mpfr_ptr buffer_next_below) noexcept {
  assert(a != dst);
  assert(dst != buffer_next_below);
  assert(a != buffer_next_below);

  // assert(mpfr_get_prec(a) == mpfr_get_prec(dst));
  // assert(mpfr_get_prec(dst) == mpfr_get_prec(buffer_next_below));
  mpfr_set_prec(dst, mpfr_get_prec(a));
  mpfr_set_prec(buffer_next_below, mpfr_get_prec(a));

  mpfr_set(dst, a, MPFR_RNDN);
  mpfr_set(buffer_next_below, a, MPFR_RNDN);

  mpfr_nextbelow(buffer_next_below);
  mpfr_sub(buffer_next_below, a, buffer_next_below, MPFR_RNDN);

  mpfr_nextabove(dst);
  mpfr_sub(dst, dst, a, MPFR_RNDN);

  mpfr_min(dst, buffer_next_below, dst, MPFR_RNDN);
}

uint32_t find_min_required_precision(boostmp::mpfr_float a,
                                     boostmp::mpfr_float& resolution,
                                     boostmp::mpfr_float& buf1,
                                     const boostmp::mpfr_float& unit) noexcept {
  const uint32_t min_prec = mpfr_min_prec(a.backend().data());
  uint32_t precision{min_prec};
  a.precision(precision);

  //  std::stringstream ss;
  //  ss << unit;
  //  std::string unit_str;
  //  ss >> unit_str;

  assert(unit > 0);
  while (true) {
    resolution_around(a.backend().data(), resolution.backend().data(),
                      buf1.backend().data());
    assert(mpfr_number_p(resolution.backend().data()));
    assert(resolution > 0);

    //    ss.clear();
    //    std::string resolution_str;
    //    ss << resolution;
    //    ss >> resolution_str;

    if (resolution > unit) {
      precision++;
      a.precision(precision);
    } else {
      break;
    }
  }

  return precision;
}

uint32_t fractal_utils::required_precision_of(
    const fractal_utils::center_wind<boostmp::mpfr_float>& wind, int rows,
    int cols) noexcept {
  assert(rows > 0);
  assert(cols > 0);
  return std::max(required_precision_of(wind.center[0], wind.x_span, cols),
                  required_precision_of(wind.center[1], wind.y_span, rows));
}