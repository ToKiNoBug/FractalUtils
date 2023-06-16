
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