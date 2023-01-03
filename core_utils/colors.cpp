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

#include "color_sources.h"
#include "fractal_colors.h"

#include <memory>

using src_t = const float (*)[3];

inline const float *interpolate(const float f, const src_t src) {
  const int offset = f * (512 - 1);
  return src[offset];
}

#define FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CASE(val)                             \
  case fractal_utils::color_series::val:                                       \
    return fractal_utils::internal::src_##val;

src_t get_source(const fractal_utils::color_series cs) noexcept {
  switch (cs) {
    FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CASE(autumn);
    FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CASE(bone);
    FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CASE(color_cube);
    FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CASE(cool);
    FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CASE(copper);
    FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CASE(flag);
    FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CASE(gray);
    FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CASE(hot);
    FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CASE(hsv);
    FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CASE(jet);
    FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CASE(lines);
    FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CASE(pink);
    FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CASE(prism);
    FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CASE(spring);
    FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CASE(winter);
    FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CASE(summer);
    FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CASE(parula);
  }

  return nullptr;
}

using namespace fractal_utils;
pixel_RGB fractal_utils::color_u8c3(const float f,
                                    const color_series cs) noexcept {
  pixel_RGB ret;
  const src_t src = get_source(cs);

  if (src != nullptr) {
    ret.value[0] = interpolate(f, src)[0] * 255;
    ret.value[1] = interpolate(f, src)[1] * 255;
    ret.value[2] = interpolate(f, src)[2] * 255;
  } else {
    ret.value[0] = 0;
    ret.value[1] = 0;
    ret.value[2] = 0;
  }

  return ret;
}

using namespace fractal_utils;
pixel_ARGB fractal_utils::color_u8c4(const float f,
                                     const color_series cs) noexcept {
  pixel_ARGB ret;
  const src_t src = get_source(cs);

  uint32_t R, G, B;

  if (src != nullptr) {
    R = interpolate(f, src)[0] * 255;
    G = interpolate(f, src)[1] * 255;
    B = interpolate(f, src)[2] * 255;
  } else {
    R = 0;
    G = 0;
    B = 0;
  }

  ret = (0xFFUL << 24) | ((R & 0xFF) << 16) | ((G & 0xFF) << 8) | (B & 0xFF);

  return ret;
}

void fractal_utils::color_u8c3_many(const float *const f, const color_series cs,
                                    const size_t pixel_num,
                                    pixel_RGB *const dest) noexcept {

  const src_t src = get_source(cs);

  if (src == nullptr) {
#ifdef __GNUC__
    __builtin_memset(dest, 0, pixel_num * sizeof(pixel_RGB));
#else
    memset(dest, 0, pixel_num * sizeof(pixel_RGB));
#endif
    return;
  }

  for (size_t pidx = 0; pidx < pixel_num; pidx++) {
    dest[pidx].value[0] = interpolate(f[pidx], src)[0] * 255;
    dest[pidx].value[1] = interpolate(f[pidx], src)[1] * 255;
    dest[pidx].value[2] = interpolate(f[pidx], src)[2] * 255;
  }
}

void fractal_utils::color_u8c4_many(const float *const f, const color_series cs,
                                    const size_t pixel_num,
                                    pixel_ARGB *const dest) noexcept {

  const src_t src = get_source(cs);

  if (src == nullptr) {
#ifdef __GNUC__
    __builtin_memset(dest, 0, pixel_num * sizeof(pixel_ARGB));
#else
    memset(dest, 0, pixel_num * sizeof(pixel_ARGB));
#endif
    return;
  }

  for (size_t pidx = 0; pidx < pixel_num; pidx++) {
    uint32_t R, G, B;
    R = interpolate(f[pidx], src)[0] * 255;
    G = interpolate(f[pidx], src)[1] * 255;
    B = interpolate(f[pidx], src)[2] * 255;
    R = R & 0xFF;
    G = G & 0xFF;
    B = B & 0xFF;

    dest[pidx] = (0xFF << 24) | (R << 16) | (G << 8) | (B);
  }
}