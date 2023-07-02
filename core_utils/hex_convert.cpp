/*
Copyright © 2022-2023  TokiNoBug
This file is part of FractalUtils.

FractalUtils is free software: you can redistribute it and/or modify it under
                                                                   the terms of
the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

                                       FractalUtils is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

`  You should have received a copy of the GNU General Public License
 along with FractalUtils.  If not, see <https://www.gnu.org/licenses/>.

 Contact with me:
 github:https://github.com/ToKiNoBug
*/

#include "hex_convert.h"

constexpr inline uint8_t hex_single_letter_to_u8(char ch) noexcept {
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  }

  if (ch >= 'A' && ch <= 'Z') {
    return ch - 'A' + 10;
  }

  if (ch >= 'a' && ch <= 'z') {
    return ch - 'a' + 10;
  }

  return 0xFF;
}

constexpr bool inline is_valid_char(char ch) noexcept {
  if (ch >= '0' && ch <= '9') {
    return true;
  }

  if (ch >= 'A' && ch <= 'Z') {
    return true;
  }

  if (ch >= 'a' && ch <= 'z') {
    return true;
  }

  return false;
}

std::optional<size_t> fractal_utils::hex_2_bin(const char *src_beg,
                                               const char *src_end, void *dst,
                                               size_t dst_capacity) noexcept {
  if (src_end - src_beg < 2) {
    return std::nullopt;
  }

  if (src_beg[0] == '0' && src_beg[1] == 'x') {
    return hex_2_bin(src_beg + 2, src_end, dst, dst_capacity);
  }

  if ((src_end - src_beg) % 2 != 0) {
    return std::nullopt;
  }

  if (dst_capacity < (src_end - src_beg) / 2) {
    return std::nullopt;
  }

  size_t offset = 0;
  uint8_t *const dst_u8 = reinterpret_cast<uint8_t *>(dst);
  for (; src_beg < src_end; src_beg += 2) {
    if (!is_valid_char(src_beg[0]) || !is_valid_char(src_beg[1])) {
      return std::nullopt;
    }

    uint8_t val = (hex_single_letter_to_u8(src_beg[0]) << 4) |
                  hex_single_letter_to_u8(src_beg[1]);
    dst_u8[offset] = val;
    offset++;
  }
  return offset;
}

std::optional<size_t> fractal_utils::hex_2_bin(std::string_view sv, void *dst,
                                               size_t dst_capacity) noexcept {
  return ::fractal_utils::hex_2_bin(sv.data(), sv.data() + sv.size(), dst,
                                    dst_capacity);
}

std::optional<size_t> fractal_utils::hex_2_bin(
    std::string_view sv, std::span<uint8_t> dest) noexcept {
  return ::fractal_utils::hex_2_bin(sv, dest.data(), dest.size());
}

constexpr inline char u8_to_hex_char(uint8_t val) noexcept {
  if (val < 10) {
    return val + '0';
  }

  return val - 10 + 'A';
}

std::optional<size_t> fractal_utils::bin_2_hex(const void *src,
                                               size_t src_bytes, char *dst,
                                               size_t dst_capacity,
                                               bool starts_with_0x) noexcept {
  size_t dst_offset = 0;
  if (dst_capacity < 2) {
    return std::nullopt;
  }

  if (dst_capacity < src_bytes * 2 + (starts_with_0x)*2) {
    return std::nullopt;
  }

  if (starts_with_0x) {
    dst[0] = '0';
    dst[1] = 'x';
    dst_offset += 2;
  }

  const uint8_t *const src_u8 = reinterpret_cast<const uint8_t *>(src);

  for (size_t src_offset = 0; src_offset < src_bytes; src_offset++) {
    dst[dst_offset] = u8_to_hex_char(src_u8[src_offset] >> 4);
    dst_offset++;
    dst[dst_offset] = u8_to_hex_char(src_u8[src_offset] & 0b1111);
    dst_offset++;
  }

  return dst_offset;
}

std::optional<size_t> fractal_utils::bin_2_hex(std::span<const uint8_t> src,
                                               std::span<char> dst,
                                               bool starts_with_0x) noexcept {
  return ::fractal_utils::bin_2_hex(src.data(), src.size(), dst.data(),
                                    dst.size(), starts_with_0x);
}