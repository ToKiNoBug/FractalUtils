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

#ifndef FRACTALUTILS_MPFR_SUPPORT_H
#define FRACTALUTILS_MPFR_SUPPORT_H

#include <mpfr.h>
#include <boost/multiprecision/mpfr.hpp>
#include <span>
#include <vector>
#include <optional>
#include <fractal_map.h>

namespace boostmp = boost::multiprecision;

namespace fractal_utils {

namespace internal {

size_t limb_num(const __mpfr_struct* number) noexcept;

size_t limb_bytes(const __mpfr_struct* number) noexcept;

consteval size_t fixed_part_bytes_of_mpfr() noexcept {
  __mpfr_struct temp;

  return sizeof(temp._mpfr_prec) + sizeof(temp._mpfr_sign) +
         sizeof(temp._mpfr_exp);
}

size_t required_bytes_of(const __mpfr_struct* number) noexcept;

size_t encode_mpfr_c_struct(const __mpfr_struct* number,
                            std::span<uint8_t> dest) noexcept;

bool decode_mpfr_c_struct(std::span<const uint8_t> code,
                          __mpfr_struct* number) noexcept;
}  // namespace internal

size_t encode_boost_mpfr_float(const boostmp::mpfr_float& num,
                               std::span<uint8_t> dest) noexcept;

std::vector<uint8_t> encode_boost_mpfr_float(
    const boostmp::mpfr_float& num) noexcept;

std::optional<boostmp::mpfr_float> decode_boost_mpfr_float(
    const std::span<const uint8_t> code) noexcept;

uint32_t required_precision_of(const boostmp::mpfr_float& center,
                               const boostmp::mpfr_float& span,
                               uint32_t num_pixels) noexcept;

uint32_t required_precision_of(
    const fractal_utils::center_wind<boostmp::mpfr_float>& wind, int rows,
    int cols) noexcept;

}  // namespace fractal_utils

#endif  // FRACTALUTILS_MPFR_SUPPORT_H
