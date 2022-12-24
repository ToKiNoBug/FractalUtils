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

#include "png_utils.h"

#include "../core_utils/core_utils.h"

#include <cstring>
#include <stdio.h>

using namespace fractal_utils;

int main() {

  // printf("rua~\n");

  fractal_map map = fractal_map::create(128, 256, 3);

  // printf("address = %p\n", map.data);

  memset(map.data, 0xAF, map.cols * map.rows * map.element_bytes);

  bool success = true;

  if (map.element_bytes == 3) {

    pixel_RGB *const data = reinterpret_cast<pixel_RGB *>(map.data);

    for (uint64_t i = 0; i < map.rows * map.cols; i++) {
      data[i].value[i % 3] = 0xFF;
    }

    success = write_png("test_u8c3.png", color_space::u8c3, map);
  }

  printf("success = %i.\n", int(success));

  return 0;
}