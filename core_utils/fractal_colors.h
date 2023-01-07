/*
 Copyright © 2022-2023  TokiNoBug
This file is part of FractalUtils.

    FractalUtils is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FractalUtils is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FractalUtils.  If not, see <https://www.gnu.org/licenses/>.

    Contact with me:
    github:https://github.com/ToKiNoBug
*/

#ifndef FRACTALUTILS_COLORS_H
#define FRACTALUTILS_COLORS_H

#include <stddef.h>
#include <stdint.h>

namespace fractal_utils {
enum class color_series : uint8_t {
  autumn,
  bone,
  color_cube,
  cool,
  copper,
  flag,
  gray,
  hot,
  hsv,
  jet,
  lines,
  pink,
  prism,
  spring,
  winter,
  summer,
  parula
};

struct pixel_RGB {
  pixel_RGB() = default;
  inline pixel_RGB(uint8_t R, uint8_t G, uint8_t B) {
    value[0] = R;
    value[1] = G;
    value[2] = B;
  }

  inline pixel_RGB(uint32_t ARGB) {
    value[0] = (ARGB & 0x00FF0000U) >> 16;
    value[1] = (ARGB & 0x0000FF00U) >> 8;
    value[1] = (ARGB & 0x000000FFU);
  }

  uint8_t value[3];
};

using pixel_ARGB = uint32_t;

pixel_RGB color_u8c3(const float f, const color_series) noexcept;

pixel_ARGB color_u8c4(const float f, const color_series) noexcept;

void color_u8c3_many(const float *const f, const color_series,
                     const size_t pixel_num, pixel_RGB *const dest) noexcept;

void color_u8c4_many(const float *const f, const color_series,
                     const size_t pixel_num, pixel_ARGB *const dest) noexcept;

}  // namespace fractal_utils

#endif  // FRACTALUTILS_COLORS_H