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

#ifndef FRACTALUTILS_FRACTAL_PNG_H
#define FRACTALUTILS_FRACTAL_PNG_H

#include "fractal_map.h"

namespace fractal_utils {

enum class color_space : uint8_t { u8c1 = 1, u8c3 = 3, u8c4 = 4 };

bool write_png(const char *const filename, const color_space cs,
               const fractal_map &map) noexcept;

bool write_png(const char *const filename, const color_space cs,
               const void *const *const row_ptrs, const uint64_t rows,
               const uint64_t cols) noexcept;
} // namespace fractal_utils

#endif // FRACTALUTILS_FRACTAL_PNG_H