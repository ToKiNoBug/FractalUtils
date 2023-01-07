#ifndef FRACTALUTILS_ZOOM_UTILS_H
#define FRACTALUTILS_ZOOM_UTILS_H

#include <QMainWindow>
#include <mutex>
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
  using compute_fractal_callback_fun_t =
      void (*)(const fractal_utils::wind_base &, void *custom_ptr,
               fractal_utils::fractal_map *map_fractal);
  using render_fractal_callback_fun_t =
      void (*)(const fractal_utils::fractal_map &map_fractal,
               const fractal_utils::wind_base &window, void *custom_ptr,
               fractal_utils::fractal_map *map_u8c3_do_not_resize);
  using export_frame_callback_fun_t =
      bool (*)(const fractal_utils::fractal_map &map_fractal,
               const fractal_utils::wind_base &window, void *custom_ptr,
               const fractal_utils::fractal_map &map_u8c3_do_not_resize,
               const char *filename);

 private:
  // this initialize function should not be invoked by other callers
  explicit zoom_utils_mainwindow(QWidget *parent,
                                 const std::array<int, 2> &window_size,
                                 int scale);

 public:
  // initialize with type of floating point
  template <typename float_t>
  explicit zoom_utils_mainwindow(
      float_t parameter_for_type_deduction, QWidget *parent = nullptr,
      const std::array<int, 2> &window_size = std::array<int, 2>({320, 320}),
      int scale = 1)
      : zoom_utils_mainwindow(
            parent, window_size,
            fractal_utils::template callback_create_center_wind<float_t>,
            fractal_utils::callback_destroy_center_wind, scale) {}

  // initialize with callbacks
  explicit zoom_utils_mainwindow(QWidget *parent,
                                 const std::array<int, 2> &window_size,
                                 create_wind_callback_fun_t cwcf,
                                 destroy_wind_callback_fun_t dwcf, int scale);

  ~zoom_utils_mainwindow();

  void set_window(const fractal_utils::wind_base &w) const noexcept {
    w.copy_to(this->window);
  }

  create_wind_callback_fun_t create_windows_function() const noexcept;
  destroy_wind_callback_fun_t destroy_windows_function() const noexcept;

  int rows() const noexcept;

  int cols() const noexcept;

  void compute_and_paint() noexcept;

  void display_range() noexcept;

 private:
  Ui::zoom_utils_mainwindow *ui;
  fractal_utils::wind_base *window{nullptr};

  std::stack<fractal_utils::wind_base *> previous_windows;

  create_wind_callback_fun_t callback_create_wind = nullptr;
  destroy_wind_callback_fun_t callback_destroy_center_wind =
      fractal_utils::callback_destroy_center_wind;

  std::mutex lock;

 public:
  compute_fractal_callback_fun_t callback_compute_fun = nullptr;
  render_fractal_callback_fun_t callback_render_fun = nullptr;
  export_frame_callback_fun_t callback_export_fun = nullptr;
  void *custom_parameters = nullptr;
  fractal_utils::fractal_map map_fractal;
  // list of file extension name seperated by double colon ;; example :
  // .tbf;;.bs_frame
  QString frame_file_extension_list = "";

  const int scale;

 private:
  QImage img_u8c3;

 public slots:

  void received_wheel_move(std::array<int, 2> pos, bool is_scaling_up);
  void received_mouse_move(std::array<int, 2> pos);

  void on_btn_revert_clicked();
  void on_btn_repaint_clicked();
  void on_btn_save_image_clicked();
  void on_btn_save_frame_clicked();
};

#endif  // FRACTALUTILS_ZOOM_UTILS_H
