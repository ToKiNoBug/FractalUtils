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

#ifndef FRACTAL_UTILS_RENDER_UTILS_RENDER_UTILS_H
#define FRACTAL_UTILS_RENDER_UTILS_RENDER_UTILS_H

#include "fractal_colors.h"
#include "fractal_map.h"
#include <functional>

namespace fractal_utils {

template <typename A, typename B, typename pixel_t> struct render_AB_options {
  size_t row_beg;
  size_t row_end;
  size_t col_beg;
  size_t col_end;
  std::function<void(A, B)> fun_counter;
  std::function<pixel_t(A, B)> fun_color;
};

template <typename A, typename B, typename pixel_t>
void render_AB(const fractal_map &mat_A, const fractal_map &mat_B,
               fractal_map &mat_img,
               const render_AB_options<A, B, pixel_t> &options) noexcept {
  assert(mat_A.element_bytes == sizeof(A));
  assert(mat_B.element_bytes == sizeof(B));
  assert(mat_img.element_bytes == sizeof(pixel_t));

  assert(mat_A.rows == mat_B.rows);
  assert(mat_B.rows == mat_img.rows);

  assert(mat_A.cols == mat_B.cols);
  assert(mat_B.cols == mat_img.cols);

  assert(options.row_beg < options.row_end);
  assert(options.row_end <= mat_A.rows);

  assert(options.col_beg < options.col_end);
  assert(options.col_end <= mat_A.cols);

  assert(bool(options.fun_color));

  if (!options.fun_counter) {
    for (size_t r = options.row_beg; r < options.row_end; r++) {
      for (size_t c = options.col_beg; c < options.col_end; c++) {
        options.fun_counter(mat_A.at<A>(r, c), mat_B.at<B>(r, c));
      }
    }
  }

  for (size_t r = options.row_beg; r < options.row_end; r++) {
    for (size_t c = options.col_beg; c < options.col_end; c++) {
      mat_img.at<pixel_t>(r, c) =
          options.fun_color(mat_A.at<A>(r, c), mat_B.at<B>(r, c));
    }
  }
}

} // namespace fractal_utils

#endif // FRACTAL_UTILS_RENDER_UTILS_RENDER_UTILS_H