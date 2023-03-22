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

void *fractal_utils::allocate_memory_aligned(size_t alignment,
                                             size_t bytes) noexcept {
#ifdef _WIN32
  return _aligned_malloc(bytes, alignment);
#else
  return aligned_alloc(alignment, bytes);
#endif
}

fractal_utils::fractal_map::fractal_map(size_t __rows, size_t __cols,
                                        uint32_t __element_bytes)
    : rows(__rows), cols(__cols), element_bytes(__element_bytes),
      call_free_on_destructor(true) {
  if (__rows <= 0 || __cols <= 0 || __element_bytes <= 0) {
    // no memory to be allocated
    this->data = nullptr;
  } else {
    this->call_free_on_destructor = true;

    this->data = allocate_memory_aligned(64, __rows * __cols * __element_bytes);
  }
}
fractal_utils::fractal_map::fractal_map(size_t __rows, size_t __cols,
                                        uint32_t __element_bytes, void *__data)
    : data(__data), rows(__rows), cols(__cols), element_bytes(__element_bytes),
      call_free_on_destructor(false) {}

fractal_map fractal_utils::fractal_map::create(size_t rows, size_t cols,
                                               size_t sizeof_element) noexcept {
  return fractal_map(rows, cols, sizeof_element);
}

fractal_utils::fractal_map::~fractal_map() {
  if (this->call_free_on_destructor && this->data != nullptr) {
    free(this->data);
  }
}

void fractal_utils::fractal_map::release() noexcept {
  if (this->call_free_on_destructor) {
    this->call_free_on_destructor = false;
    free(this->data);
  }
  this->data = nullptr;
}

fractal_utils::fractal_map::fractal_map(const fractal_map &src)
    : rows(src.rows), cols(src.cols), element_bytes(src.element_bytes),
      call_free_on_destructor(true) {
  this->data = allocate_memory_aligned(64, src.byte_count());
#ifdef __GNUC__
  __builtin_memcpy(this->data, src.data, src.byte_count());
#else
  memcpy(this->data, src.data, src.byte_count());
#endif
}

fractal_utils::fractal_map::fractal_map(fractal_map &&src)
    : rows(src.rows), cols(src.cols), element_bytes(src.element_bytes) {
  this->data = src.data;
  this->call_free_on_destructor = src.call_free_on_destructor;

  src.data = nullptr;
  src.call_free_on_destructor = false;
  // src.element_bytes = 0;
}
