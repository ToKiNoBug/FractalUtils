//
// Created by joseph on 6/17/23.
//

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