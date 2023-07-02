/*
  Copyright © 2022-2023  TokiNoBug
  This file is part of FractalUtils.

  FractalUtils is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  FractalUtils is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

`  You should have received a copy of the GNU General Public License
   along with FractalUtils.  If not, see <https://www.gnu.org/licenses/>.

   Contact with me:
   github:https://github.com/ToKiNoBug
*/

#ifndef FRACTALUTILS_CENTER_WIND_HPP
#define FRACTALUTILS_CENTER_WIND_HPP

#include <array>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <typeinfo>
#include <sstream>
#include <span>
#include <cstdlib>

#if defined(__GNUC__) && !defined(__clang__)
#include <quadmath.h>
#endif

namespace fractal_utils {

class wind_base {
 public:
  virtual ~wind_base() = default;

  virtual bool copy_to(wind_base *const dest) const noexcept = 0;

  [[nodiscard]] virtual std::array<double, 2> displayed_center()
      const noexcept = 0;
  [[nodiscard]] virtual double displayed_x_span() const noexcept = 0;
  [[nodiscard]] virtual double displayed_y_span() const noexcept = 0;

  [[nodiscard]] virtual std::array<double, 2> displayed_coordinate(
      const std::array<int, 2> &total_size_row_col,
      const std::array<int, 2> &position_row_col) const noexcept = 0;

  [[nodiscard]] virtual std::array<double, 2> displayed_left_top_corner()
      const noexcept = 0;
  [[nodiscard]] virtual std::array<double, 2> displayed_right_bottom_corner()
      const noexcept = 0;

  [[nodiscard]] virtual std::string x_span_string(
      std::stringstream &ss) const noexcept = 0;
  [[nodiscard]] virtual std::string y_span_string(
      std::stringstream &ss) const noexcept = 0;
  [[nodiscard]] virtual std::array<std::string, 2> center_string(
      std::stringstream &ss) const noexcept = 0;
  [[nodiscard]] virtual std::array<std::string, 2> coordinate_string(
      const std::array<int, 2> &total_size_row_col,
      const std::array<int, 2> &position_row_col,
      std::stringstream &ss) const noexcept = 0;
  [[nodiscard]] virtual std::array<std::string, 2> left_top_corner_string(
      std::stringstream &ss) const noexcept = 0;
  [[nodiscard]] virtual std::array<std::string, 2> right_bottom_corner_string(
      std::stringstream &ss) const noexcept = 0;
  [[nodiscard]] virtual std::array<std::string, 2> left_bottom_corner_string(
      std::stringstream &ss) const noexcept = 0;
  [[nodiscard]] virtual std::array<std::string, 2> right_top_corner_string(
      std::stringstream &ss) const noexcept = 0;

  virtual void update_center(const std::array<int, 2> &total_size_row_col,
                             const std::array<int, 2> &position_row_col,
                             double zoom_ratio) & noexcept = 0;

  virtual void update_scale(double zoom_ratio, int zoom_times) & noexcept = 0;

  virtual void *center_data(size_t *bytes = nullptr) noexcept = 0;
  virtual const void *center_data(size_t *bytes = nullptr) const noexcept = 0;

  virtual void set_x_span(double __x_span) & noexcept = 0;
  virtual void set_y_span(double __y_span) & noexcept = 0;

  [[nodiscard]] virtual bool set_x_span(std::string_view sv,
                                        std::stringstream &ss) & noexcept = 0;
  [[nodiscard]] virtual bool set_y_span(std::string_view sv,
                                        std::stringstream &ss) & noexcept = 0;

  [[nodiscard]] virtual size_t float_size() const noexcept = 0;

  [[nodiscard]] virtual const std::type_info &float_typeinfo()
      const noexcept = 0;

  template <typename T>
  inline bool float_type_matches() const noexcept {
    return typeid(T) == this->float_typeinfo();
  }

  [[nodiscard]] virtual wind_base *create_another() const noexcept = 0;

  virtual bool operator==(const wind_base &another) const noexcept = 0;

  inline bool operator!=(const wind_base &another) const noexcept {
    return !operator==(another);
  }
};

template <typename float_t>
class center_wind : public wind_base {
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

  inline std::array<float_t, 2> left_bottom_corner() const noexcept {
    std::array<float_t, 2> ret = this->center;

    ret[idx_x] -= x_span / 2;
    ret[idx_y] -= y_span / 2;

    return ret;
  }

  inline std::array<float_t, 2> right_top_corner() const noexcept {
    std::array<float_t, 2> ret = this->center;

    ret[idx_x] += x_span / 2;
    ret[idx_y] += y_span / 2;

    return ret;
  }

  [[nodiscard]] std::array<double, 2> displayed_left_top_corner()
      const noexcept override {
    std::array<float_t, 2> temp = this->left_top_corner();
    return {double(temp[0]), double(temp[1])};
  }

  [[nodiscard]] std::array<double, 2> displayed_right_bottom_corner()
      const noexcept override {
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

  [[nodiscard]] std::array<double, 2> displayed_center()
      const noexcept override {
    std::array<double, 2> ret;
    ret[0] = double(this->center[0]);
    ret[1] = double(this->center[1]);
    return ret;
  }

  [[nodiscard]] double displayed_x_span() const noexcept override {
    return double(this->x_span);
  }

  [[nodiscard]] double displayed_y_span() const noexcept override {
    return double(this->y_span);
  }

  [[nodiscard]] std::array<double, 2> displayed_coordinate(
      const std::array<int, 2> &total_size,  //[row,col]
      const std::array<int, 2> &position     //[row,col]
  ) const noexcept override {
    assert(total_size[0] > 0 && total_size[1] > 0);
    // assert(position[0] >= 0 && position[0] < total_size[0]);

    std::array<double, 2> ret = this->displayed_center();

    //[row,col] in range (0,1)
    std::array<double, 2> relative_offset_rc{0, 0};
    for (int idx = 0; idx < 2; idx++) {
      relative_offset_rc[idx] = (position[idx] + 0.5) / (total_size[idx]) - 0.5;
    }

    ret[0] += relative_offset_rc[1] * double(this->x_span);
    ret[1] -= relative_offset_rc[0] * double(this->y_span);
    return ret;
  }

  [[nodiscard]] static inline std::string format_value(
      const float_t &val, std::stringstream &ss) noexcept {
#ifdef __GNUC__
    constexpr bool is_float128 = std::is_same_v<float_t, __float128>;
#else
    constexpr bool is_float128 = false;
#endif

    std::string ret;
    if constexpr (is_float128) {
#ifndef __clang__
      ret.resize(4096);
      const int sz = quadmath_snprintf(ret.data(), ret.size(), "%Qf", val);
      ret.resize(sz);
#else
      ss.clear();
      ss << double(val);
      ss >> ret;
#endif
    } else {
      ss.clear();
      ss << val;
      ss >> ret;
    }
    return ret;
  }
  [[nodiscard]] static inline std::array<std::string, 2> format_array_2(
      std::span<const float_t, 2> val, std::stringstream &ss) noexcept {
    std::array<std::string, 2> ret;
    ret[0] = format_value(val[0], ss);
    ret[1] = format_value(val[1], ss);
    return ret;
  }

  [[nodiscard]] std::string x_span_string(
      std::stringstream &ss) const noexcept override {
    return format_value(this->x_span, ss);
  }
  [[nodiscard]] std::string y_span_string(
      std::stringstream &ss) const noexcept override {
    return format_value(this->y_span, ss);
  }
  [[nodiscard]] std::array<std::string, 2> center_string(
      std::stringstream &ss) const noexcept override {
    return format_array_2(this->center, ss);
  }

  [[nodiscard]] std::array<std::string, 2> coordinate_string(
      const std::array<int, 2> &total_size_rc,
      const std::array<int, 2> &position_rc,
      std::stringstream &ss) const noexcept override {
    assert(total_size_rc[0] > 0 && total_size_rc[1] > 0);
    // assert(position[0] >= 0 && position[0] < total_size[0]);

    std::array<float_t, 2> result = this->center;

    //[row,col] in range (0,1)
    std::array<double, 2> relative_offset_rc{0, 0};
    for (int idx = 0; idx < 2; idx++) {
      relative_offset_rc[idx] =
          (position_rc[idx] + 0.5) / (position_rc[idx]) - 0.5;
    }

    result[0] += this->x_span * relative_offset_rc[1];
    result[1] -= this->y_span * relative_offset_rc[0];
    return format_array_2(result, ss);
  }

  [[nodiscard]] std::array<std::string, 2> left_top_corner_string(
      std::stringstream &ss) const noexcept override {
    return format_array_2(this->left_top_corner(), ss);
  }

  [[nodiscard]] std::array<std::string, 2> right_bottom_corner_string(
      std::stringstream &ss) const noexcept override {
    return format_array_2(this->right_bottom_corner(), ss);
  }

  [[nodiscard]] std::array<std::string, 2> left_bottom_corner_string(
      std::stringstream &ss) const noexcept override {
    return format_array_2(this->left_bottom_corner(), ss);
  }

  [[nodiscard]] std::array<std::string, 2> right_top_corner_string(
      std::stringstream &ss) const noexcept override {
    return format_array_2(this->right_top_corner(), ss);
  }

  void update_center(const std::array<int, 2> &total_size,
                     const std::array<int, 2> &position,
                     double zoom_ratio) & noexcept override {
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

  void update_scale(double zoom_ratio, int zoom_times) & noexcept override {
    assert(zoom_ratio == zoom_ratio);
    assert(zoom_ratio != 0);
    if (zoom_times < 0) {
      zoom_ratio = 1 / zoom_ratio;
      zoom_times = -zoom_times;
    }

    for (int zi = 0; zi < zoom_times; zi++) {
      this->x_span /= zoom_ratio;
      this->y_span /= zoom_ratio;
    }
  }

  [[nodiscard]] void *center_data(size_t *bytes = nullptr) noexcept override {
    if (bytes != nullptr) {
      *bytes = sizeof(this->center);
    }

    return this->center.data();
  }

  [[nodiscard]] const void *center_data(
      size_t *bytes = nullptr) const noexcept override {
    if (bytes != nullptr) {
      *bytes = sizeof(this->center);
    }

    return this->center.data();
  }

  void set_x_span(double _x_span) & noexcept override {
    this->x_span = float_t(_x_span);
  }

  void set_y_span(double _y_span) & noexcept override {
    this->y_span = float_t(_y_span);
  }

  [[nodiscard]] static bool scan_float_t(std::string_view sv,
                                         std::stringstream &ss,
                                         float_t &f) noexcept {
    ss.clear();
#ifdef __GNUC__
    constexpr bool is_quadmath = std::is_same_v<float_t, __float128>;
#ifdef __clang__
    constexpr bool is_clang = true;
#else
    constexpr bool is_clang = false;
#endif
#else
    constexpr bool is_quadmath = false;
#endif
    if constexpr (is_quadmath) {
#ifndef __clang__
      f = strtof128(sv.data(), nullptr);
#else
      ss << sv;
      long double temp;
      ss >> temp;
      f = temp;
#endif
    } else {
      ss << sv;
      ss >> f;
    }
    return true;
  }

  [[nodiscard]] bool set_x_span(std::string_view sv,
                                std::stringstream &ss) & noexcept override {
    return scan_float_t(sv, ss, this->x_span);
  }

  [[nodiscard]] bool set_y_span(std::string_view sv,
                                std::stringstream &ss) & noexcept override {
    return scan_float_t(sv, ss, this->y_span);
  }

  [[nodiscard]] size_t float_size() const noexcept override {
    return sizeof(float_t);
  }

  [[nodiscard]] const std::type_info &float_typeinfo() const noexcept override {
    return typeid(float_t);
  }

  [[nodiscard]] wind_base *create_another() const noexcept override {
    return new center_wind<float_t>;
  }

  bool operator==(const wind_base &another) const noexcept override {
    auto anotherp = dynamic_cast<const center_wind<float_t> *>(&another);
    if (anotherp == nullptr) {
      return false;
    }

    return (this->center[0] == anotherp->center[0]) &&
           (this->center[1] == anotherp->center[1]) &&
           (this->x_span == anotherp->x_span) &&
           (this->y_span == anotherp->y_span);
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

}  // namespace fractal_utils

#endif  // FRACTALUTILS_CENTER_WIND_HPP
