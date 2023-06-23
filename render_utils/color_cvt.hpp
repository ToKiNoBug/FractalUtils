//
// Created by David on 2023/6/23.
//

#ifndef FRACTALUTILS_COLOR_CVT_HPP
#define FRACTALUTILS_COLOR_CVT_HPP

#include <cassert>
#include <cmath>

#ifdef __CUDA__
#define FRACTAL_UTILS_CUDA_HOST_DEVICE_FUN __host__ __device__
#else
#define FRACTAL_UTILS_CUDA_HOST_DEVICE_FUN
#endif

namespace fractal_utils {

inline FRACTAL_UTILS_CUDA_HOST_DEVICE_FUN void hsv_to_rgb(float H, float S,
                                                          float V, float &R,
                                                          float &G,
                                                          float &B) noexcept {
  assert(H >= 0 && H < 360);
  assert(S >= 0 && S <= 1);
  assert(V >= 0 && V <= 1);

  const float C = V * S;

  const int H_i = H;

  const int H_mod_60 = H_i / 60;

  const float X = C * (1 - std::abs((H_i / 60) % 2 - 1));

  float results[3][6];

  results[0] = {C, X, 0};
  results[1] = {X, C, 0};
  results[2] = {0, C, X};
  results[3] = {0, X, C};
  results[4] = {X, 0, C};
  results[5] = {C, 0, X};

  const float *const temp = result + H_mod_60;
  const float m = V - C;
  R = temp[0] + m;
  G = temp[1] + m;
  B = temp[2] + m;
}

}  // namespace fractal_utils
#endif  // FRACTALUTILS_COLOR_CVT_HPP
