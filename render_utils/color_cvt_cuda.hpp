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

#ifndef FRACTALUTILS_COLOR_CVT_CUDA_HPP
#define FRACTALUTILS_COLOR_CVT_CUDA_HPP

#include <cassert>

#ifndef __CUDACC__
#error "This header file should only be included in a cuda source file."
#endif

#include "color_cvt.hpp"

namespace fractal_utils {

namespace cuda {

inline __device__ float3 hsv_to_rgb(const float3 rgb) noexcept {
  float3 ret;
  hsv_to_rgb(rgb.x, rgb.y, rgb.z, ret.x, ret.y, ret.z);
  return ret;
}

inline __device__ uchar3 rgb_to_u8c3(const float3 rgb) noexcept {
  assert(rgb.x >= 0 && rgb.x <= 1);
  assert(rgb.y >= 0 && rgb.y <= 1);
  assert(rgb.z >= 0 && rgb.z <= 1);

  uchar3 ret;
  ret.x = rgb.x * 255;
  ret.y = rgb.y * 255;
  ret.z = rgb.z * 255;

  return ret;
}
}  // namespace cuda

}  // namespace fractal_utils

#endif  // FRACTALUTILS_COLOR_CVT_CUDA_HPP
