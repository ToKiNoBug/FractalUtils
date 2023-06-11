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

  template <typename T> inline T &at(size_t idx) noexcept {
    assert(idx < this->rows * this->cols);

    return reinterpret_cast<T *>(this->data)[idx];
  }

  template <typename T> inline const T &at(size_t idx) const noexcept {
    assert(idx < this->rows * this->cols);

    return reinterpret_cast<T *>(this->data)[idx];
  }

  template <typename T> inline T &at(size_t r, size_t c) noexcept {
    assert(r < this->rows && c < this->cols);

    return this->at<T>(r * this->cols + c);
  }

  template <typename T> inline const T &at(size_t r, size_t c) const noexcept {
    assert(r < this->rows && c < this->cols);

    return this->at<T>(r * this->cols + c);
  }

  template <typename T> inline T *address(size_t idx) noexcept {
    assert(idx < this->rows * this->cols);
    return reinterpret_cast<T *>(this->data) + idx;
  }

  template <typename T> inline const T *address(size_t idx) const noexcept {
    assert(idx < this->rows * this->cols);
    return reinterpret_cast<const T *>(this->data) + idx;
  }

  template <typename T> inline T *address(size_t r, size_t c) noexcept {
    return this->address<T>(r * this->cols + c);
  }

  template <typename T>
  inline const T *address(size_t r, size_t c) const noexcept {
    return this->address<T>(r * this->cols + c);
  }

#ifdef FRACTAL_UTILS_HAVE_CXX_20
  template <typename T> inline std::ranges::subrange<T *> items() noexcept {
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

class wind_base {
public:
  virtual ~wind_base() = default;

  virtual bool copy_to(wind_base *const dest) const noexcept = 0;

  virtual std::array<double, 2> displayed_center() const noexcept = 0;
  virtual double displayed_x_span() const noexcept = 0;
  virtual double displayed_y_span() const noexcept = 0;

  virtual std::array<double, 2> displayed_coordinate(
      const std::array<int, 2> &total_size_row_col,
      const std::array<int, 2> &position_row_col) const noexcept = 0;

  virtual std::array<double, 2> displayed_left_top_corner() const noexcept = 0;
  virtual std::array<double, 2>
  displayed_right_bottom_corner() const noexcept = 0;

  virtual void update_center(const std::array<int, 2> &total_size_row_col,
                             const std::array<int, 2> &position_row_col,
                             double zoom_ratio) noexcept = 0;
  virtual void *center_data(size_t *bytes = nullptr) noexcept = 0;
  virtual const void *center_data(size_t *bytes = nullptr) const noexcept = 0;

  virtual void set_x_span(double __x_span) noexcept = 0;
  virtual void set_y_span(double __y_span) noexcept = 0;

  virtual size_t float_size() const noexcept = 0;

  virtual const std::type_info &float_typeinfo() const noexcept = 0;

  template <typename T> inline bool float_type_matches() const noexcept {
    return typeid(T) == this->float_typeinfo();
  }
};

template <typename float_t> class center_wind : public wind_base {
public:
  std::array<float_t, 2> center;
  float_t x_span;
  float_t y_span;

  static constexpr int idx_x = 0;
  static constexpr int idx_y = 1;

  inline std::array<float_t, 2> left_top_corner() const noexcept {
    std::array<float_t, 2> ret = this->center;

    ret[idx_x] -= x_span / 2;
    ret[idx_y] += y_span / 2;

    return ret;
  }

  inline std::array<float_t, 2> right_bottom_corner() const noexcept {
    std::array<float_t, 2> ret = this->center;

    ret[idx_x] += x_span / 2;
    ret[idx_y] -= y_span / 2;

    return ret;
  }

  std::array<double, 2> displayed_left_top_corner() const noexcept override {
    std::array<float_t, 2> temp = this->left_top_corner();
    return {double(temp[0]), double(temp[1])};
  }

  std::array<double, 2>
  displayed_right_bottom_corner() const noexcept override {
    std::array<float_t, 2> temp = this->right_bottom_corner();
    return {double(temp[0]), double(temp[1])};
  }

  bool copy_to(wind_base *const __dest) const noexcept override {
    center_wind *const dest = dynamic_cast<center_wind *>(__dest);

    if (dest == nullptr) {
      return false;
    }

    *dest = *this;
    return true;
  }

  std::array<double, 2> displayed_center() const noexcept override {
    std::array<double, 2> ret;
    ret[0] = double(this->center[0]);
    ret[1] = double(this->center[1]);
    return ret;
  }

  double displayed_x_span() const noexcept override {
    return double(this->x_span);
  }

  double displayed_y_span() const noexcept override {
    return double(this->y_span);
  }

  std::array<double, 2>
  displayed_coordinate(const std::array<int, 2> &total_size, //[row,col]
                       const std::array<int, 2> &position    //[row,col]
  ) const noexcept override {
    assert(total_size[0] > 0 && total_size[1] > 0);
    // assert(position[0] >= 0 && position[0] < total_size[0]);

    std::array<double, 2> ret = this->displayed_center();

    //[row,col] in range (0,1)
    std::array<double, 2> relative_offset_rc;
    for (int idx = 0; idx < 2; idx++) {
      relative_offset_rc[idx] = (position[idx] + 0.5) / (total_size[idx]) - 0.5;
    }

    ret[0] += relative_offset_rc[1] * double(this->x_span);
    ret[1] -= relative_offset_rc[0] * double(this->y_span);
    return ret;
  }

  void update_center(const std::array<int, 2> &total_size,
                     const std::array<int, 2> &position,
                     double zoom_ratio) noexcept override {
    assert(total_size[0] > 0 && total_size[1] > 0);
    // assert(position[0] >= 0 && position[0] < total_size[0]);

    // std::array<double, 2> ret = this->displayed_center();

    //[row,col] in range (0,1)
    std::array<float_t, 2> relative_offset_rc;
    for (int idx = 0; idx < 2; idx++) {
      relative_offset_rc[idx] =
          (position[idx] + float_t(0.5)) / (total_size[idx]) - float_t(0.5);
    }

    this->center[0] += relative_offset_rc[1] * this->x_span;
    this->center[1] -= relative_offset_rc[0] * this->y_span;

    this->x_span /= zoom_ratio;
    this->y_span /= zoom_ratio;
  }
  void *center_data(size_t *bytes = nullptr) noexcept override {
    if (bytes != nullptr) {
      *bytes = sizeof(this->center);
    }

    return this->center.data();
  }

  const void *center_data(size_t *bytes = nullptr) const noexcept override {
    if (bytes != nullptr) {
      *bytes = sizeof(this->center);
    }

    return this->center.data();
  }

  void set_x_span(double __x_span) noexcept override {
    this->x_span = float_t(__x_span);
  }

  void set_y_span(double __y_span) noexcept override {
    this->y_span = float_t(__y_span);
  }

  size_t float_size() const noexcept override { return sizeof(float_t); }

  const std::type_info &float_typeinfo() const noexcept override {
    return typeid(float_t);
  }
};

int compute_skip_indices(int size, double ratio, int png_count,
                         int png_index) noexcept;

inline int skip_rows(int full_rows, double ratio, int png_count,
                     int png_index) noexcept {
  return compute_skip_indices(full_rows, ratio, png_count, png_index);
}

inline int skip_cols(int full_cols, double ratio, int png_count,
                     int png_index) noexcept {
  return compute_skip_indices(full_cols, ratio, png_count, png_index);
}

} // namespace fractal_utils

#endif
