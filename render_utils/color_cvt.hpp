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

#ifndef FRACTALUTILS_COLOR_CVT_HPP
#define FRACTALUTILS_COLOR_CVT_HPP

#include <cassert>
#include <cmath>

#ifdef __CUDACC__
#define FRACTAL_UTILS_CUDA_HOST_DEVICE_FUN __host__ __device__
#else
#define FRACTAL_UTILS_CUDA_HOST_DEVICE_FUN
#endif

namespace fractal_utils {

namespace internal {
constexpr inline FRACTAL_UTILS_CUDA_HOST_DEVICE_FUN float float_point_mod(
    float f, int mod) noexcept {
  if (mod <= 0) {
    return NAN;
  }
  if (f >= 0) {
    while (f >= mod) {
      f -= mod;
    }
  } else {
    while (f < 0) {
      f += mod;
    }
  }

  return f;
}

constexpr inline FRACTAL_UTILS_CUDA_HOST_DEVICE_FUN float
normalize_angle_to_360(float angle) noexcept {
  return float_point_mod(angle, 360);
}

}  // namespace internal

inline FRACTAL_UTILS_CUDA_HOST_DEVICE_FUN void hsv_to_rgb(float H, float S,
                                                          float V, float &R,
                                                          float &G,
                                                          float &B) noexcept {
  H = internal::normalize_angle_to_360(H);
  assert(H >= 0 && H < 360);
  assert(S >= 0 && S <= 1);
  assert(V >= 0 && V <= 1);

  const float C = V * S;

  const int H_i = H;

  const int H_div_60 = H_i / 60;

  const float X = C * (1 - std::abs(internal::float_point_mod(H / 60, 2) - 1));

  using float3_t = float[3];

  float3_t results[6]{{C, X, 0}, {X, C, 0}, {0, C, X},
                      {0, X, C}, {X, 0, C}, {C, 0, X}};

  const float *const temp = results[H_div_60];
  const float m = V - C;
  R = temp[0] + m;
  G = temp[1] + m;
  B = temp[2] + m;
}

}  // namespace fractal_utils
#endif  // FRACTALUTILS_COLOR_CVT_HPP
