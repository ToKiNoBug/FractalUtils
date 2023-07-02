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

#ifndef FRACTALUTILS_FRACTAL_MAP_H
#define FRACTALUTILS_FRACTAL_MAP_H

#include <array>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <type_traits>
#include <typeinfo>
#include "center_wind.hpp"

#ifdef FRACTAL_UTILS_HAVE_CXX_20
#include <ranges>
#endif

namespace fractal_utils {

void *allocate_memory_aligned(size_t alignment, size_t bytes) noexcept;
void free_memory_aligned(void *data) noexcept;

class fractal_map {
 public:
  void *data{nullptr};
  const size_t rows{0};
  const size_t cols{0};
  const uint32_t element_bytes{1};

 private:
  bool call_free_on_destructor{false};

 public:
  [[nodiscard]] static fractal_map create(size_t rows, size_t cols,
                                          size_t sizeof_element) noexcept;

  ~fractal_map();
  fractal_map() = delete;
  fractal_map(const fractal_map &);
  fractal_map(fractal_map &&src);
  fractal_map(size_t __rows, size_t __cols, uint32_t __element_bytes);
  fractal_map(size_t __rows, size_t __cols, uint32_t __element_bytes,
              void *__data);

  // fractal_map &operator=(fractal_map &&src) noexcept;

  void release() noexcept;

  inline bool own_memory() const noexcept {
    return this->call_free_on_destructor;
  }

  inline size_t element_count() const noexcept {
    return this->rows * this->cols;
  }

  inline size_t byte_count() const noexcept {
    return this->rows * this->cols * this->element_bytes;
  }

  template <typename T>
  inline T &at(size_t idx) noexcept {
    assert(idx < this->rows * this->cols);

    return reinterpret_cast<T *>(this->data)[idx];
  }

  template <typename T>
  inline const T &at(size_t idx) const noexcept {
    assert(idx < this->rows * this->cols);

    return reinterpret_cast<T *>(this->data)[idx];
  }

  template <typename T>
  inline T &at(size_t r, size_t c) noexcept {
    assert(r < this->rows && c < this->cols);

    return this->at<T>(r * this->cols + c);
  }

  template <typename T>
  inline const T &at(size_t r, size_t c) const noexcept {
    assert(r < this->rows && c < this->cols);

    return this->at<T>(r * this->cols + c);
  }

  template <typename T>
  inline T *address(size_t idx) noexcept {
    assert(idx < this->rows * this->cols);
    return reinterpret_cast<T *>(this->data) + idx;
  }

  template <typename T>
  inline const T *address(size_t idx) const noexcept {
    assert(idx < this->rows * this->cols);
    return reinterpret_cast<const T *>(this->data) + idx;
  }

  template <typename T>
  inline T *address(size_t r, size_t c) noexcept {
    return this->address<T>(r * this->cols + c);
  }

  template <typename T>
  inline const T *address(size_t r, size_t c) const noexcept {
    return this->address<T>(r * this->cols + c);
  }

#ifdef FRACTAL_UTILS_HAVE_CXX_20
  template <typename T>
  inline std::ranges::subrange<T *> items() noexcept {
    assert(sizeof(T) == this->element_bytes);
    return std::ranges::subrange<T *>(
        this->address<T>(0), this->address<T>(0) + this->element_count());
  }

  template <typename T>
  inline std::ranges::subrange<const T *> items() const noexcept {
    assert(sizeof(T) == this->element_bytes);
    return std::ranges::subrange<const T *>(
        this->address<T>(0), this->address<T>(0) + this->element_count());
  }
#endif
};

}  // namespace fractal_utils

#endif
