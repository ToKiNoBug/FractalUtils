//
// Created by David on 2023/6/23.
//

#ifndef FRACTALUTILS_COLOR_CVT_CUDA_HPP
#define FRACTALUTILS_COLOR_CVT_CUDA_HPP

#include <cassert>

#ifndef __CUDA__
#error "This header file should only be included in a cuda source file."
#endif

#include "color_cvt_cuda.hpp"

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
