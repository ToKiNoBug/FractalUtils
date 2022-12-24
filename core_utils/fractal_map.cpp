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

#include "fractal_map.h"

#include <cstdlib>
#include <memory>

using namespace fractal_utils;

fractal_map fractal_utils::fractal_map::create(size_t rows, size_t cols,
                                               size_t sizeof_element) noexcept {
  fractal_map map;

  map.rows = rows;
  map.cols = cols;
  map.element_bytes = sizeof_element;

  if (rows <= 0 || cols <= 0 || sizeof_element <= 0) {
    // no memory to be allocated
    map.data = nullptr;
  } else {

    map.call_free_on_destructor = true;

    map.data =
#ifdef _WIN32
        _aligned_malloc(rows * cols * sizeof_element, 64);
#else
        aligned_alloc(64, rows * cols * sizeof_element);
#endif
  }

  return map;
}

fractal_utils::fractal_map::~fractal_map() {
  if (this->call_free_on_destructor && this->data != nullptr) {
    free(this->data);
  }
}