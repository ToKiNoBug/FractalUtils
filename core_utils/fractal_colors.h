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
  summer
};

struct pixel_RGB {
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