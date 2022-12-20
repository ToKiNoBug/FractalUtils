#ifndef FRACTALUTILS_FRACTAL_MAP_H
#define FRACTALUTILS_FRACTAL_MAP_H

#include <stddef.h>
#include <stdint.h>

#include <type_traits>

#include <array>

#include <assert.h>

namespace fractal_utils {

template <typename T>
using fast_const_t = std::conditional_t<(sizeof(T) > 8), const T &, T>;

class fractal_map {
public:
  void *data{nullptr};
  size_t rows{0};
  size_t cols{0};
  uint32_t element_bytes{1};

private:
  bool call_free_on_destructor{false};

public:
  [[nodiscard]] static fractal_map create(size_t rows, size_t cols,
                                          size_t sizeof_element) noexcept;

  ~fractal_map();

  template <typename T> inline T &at(size_t idx) noexcept {
    assert(idx < this->rows * this->cols);

    return reinterpret_cast<T *>(this->data)[idx];
  }

  template <typename T>
  inline const fast_const_t<T> &at(size_t idx) const noexcept {
    assert(idx < this->rows * this->cols);

    return reinterpret_cast<T *>(this->data)[idx];
  }

  template <typename T> inline T &at(size_t r, size_t c) noexcept {
    assert(r < this->rows && c < this->cols);

    return this->at<T>(r * this->cols + c);
  }

  template <typename T> inline auto at(size_t r, size_t c) const noexcept {
    assert(r < this->rows && c < this->cols);

    return this->at<T>(r * this->cols + c);
  }
};

template <typename float_t> class center_wind {
public:
  std::array<float_t, 2> center;
  float_t x_span;
  float_t y_span;

  static constexpr int idx_x = 0;
  static constexpr int idx_y = 1;

  inline std::array<float_t, 2> left_top_corner() const noexcept {
    std::array<float_t, 2> ret = this->center;

    ret[idx_x] -= x_span / 2;
    ret[idx_y] -= y_span / 2;

    return ret;
  }

  inline std::array<float_t, 2> right_bottom_corner() const noexcept {
    std::array<float_t, 2> ret = this->center;

    ret[idx_x] += x_span / 2;
    ret[idx_y] += y_span / 2;

    return ret;
  }
};

} // namespace fractal_utils

#endif