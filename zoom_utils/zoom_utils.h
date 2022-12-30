#ifndef FRACTALUTILS_ZOOM_UTILS_H
#define FRACTALUTILS_ZOOM_UTILS_H

#include <QMainWindow>
#include <stack>

#include "core_utils.h"

class zoom_utils_mainwindow;

namespace fractal_utils {
using mainwindow = ::zoom_utils_mainwindow;

template <typename float_t>
[[nodiscard]] wind_base *callback_create_center_wind() {
  return new center_wind<float_t>;
}

void callback_destroy_center_wind(wind_base *const w);
}  // namespace fractal_utils

// Qt mainwindow class

namespace Ui {
class zoom_utils_mainwindow;
}

class zoom_utils_mainwindow : public QMainWindow {
  Q_OBJECT
 public:
  using create_wind_callback_fun_t = fractal_utils::wind_base *(*)();
  using destroy_wind_callback_fun_t = void (*)(fractal_utils::wind_base *const);

 private:
  // this initialize function should not be invoked by other callers
  explicit zoom_utils_mainwindow(QWidget *parent = nullptr);

 public:
  // initialize with type of floating point
  template <typename float_t>
  explicit zoom_utils_mainwindow(
      QWidget *parent = nullptr,
      const std::array<int, 2> &window_size = std::array<int, 2>({320, 320}))
      : zoom_utils_mainwindow(
            parent, window_size,
            fractal_utils::template callback_create_center_wind<float_t>,
            fractal_utils::callback_destroy_center_wind) {}

  // initialize with callbacks
  explicit zoom_utils_mainwindow(QWidget *parent,
                                 const std::array<int, 2> &window_size,
                                 create_wind_callback_fun_t cwcf,
                                 destroy_wind_callback_fun_t dwcf);

  ~zoom_utils_mainwindow();

  void set_window(const fractal_utils::wind_base &w) const noexcept {
    w.copy_to(this->window);
  }

  create_wind_callback_fun_t create_windows_function() const noexcept;
  destroy_wind_callback_fun_t destroy_windows_function() const noexcept;

 private:
  Ui::zoom_utils_mainwindow *ui;
  fractal_utils::wind_base *window{nullptr};

  std::stack<fractal_utils::wind_base *> previous_windows;

  create_wind_callback_fun_t callback_create_wind = nullptr;
  destroy_wind_callback_fun_t callback_destroy_center_wind =
      fractal_utils::callback_destroy_center_wind;
};

#endif  // FRACTALUTILS_ZOOM_UTILS_H
