#ifndef FRACTALUTILS_ZOOM_UTILS_H
#define FRACTALUTILS_ZOOM_UTILS_H

#include "core_utils.h"
#include "zoom_window.h"
#include <QMainWindow>
#include <mutex>
#include <stack>
#include <string_view>

class zoom_utils_mainwindow;

namespace fractal_utils {
using mainwindow = ::zoom_utils_mainwindow;

template <typename float_t>
[[nodiscard]] wind_base *callback_create_center_wind() {
  return new center_wind<float_t>;
}

void callback_destroy_center_wind(wind_base *const w);

std::string default_hex_encode_fun(const fractal_utils::wind_base &wind_src,
                                   std::string &err);

void default_hex_decode_fun(std::string_view hex,
                            fractal_utils::wind_base &wind_dest,
                            std::string &err);

} // namespace fractal_utils

class zoom_utils_mainwindow : public fractal_utils::zoom_window {
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

  using hex_encode_fun_t = std::string (*)(
      const fractal_utils::wind_base &wind_src, std::string &err);
  using hex_decode_fun_t = void (*)(std::string_view hex,
                                    fractal_utils::wind_base &wind_dest,
                                    std::string &err);

private:
  // this initialize function should not be invoked by other callers
  explicit zoom_utils_mainwindow(QWidget *parent,
                                 const std::array<int, 2> &window_size,
                                 uint32_t map_fractal_element_size, int scale);

public:
  // initialize with type of floating point
  template <typename float_t>
  explicit zoom_utils_mainwindow(float_t parameter_for_type_deduction,
                                 QWidget *parent,
                                 const std::array<int, 2> &window_size,
                                 uint32_t map_fractal_element_size,
                                 int scale = 1)
      : zoom_utils_mainwindow(
            parent, window_size, map_fractal_element_size,
            fractal_utils::template callback_create_center_wind<float_t>,
            fractal_utils::callback_destroy_center_wind, scale) {}

  // initialize with callbacks
  explicit zoom_utils_mainwindow(QWidget *parent,
                                 const std::array<int, 2> &window_size,
                                 uint32_t map_fractal_element_size,
                                 create_wind_callback_fun_t cwcf,
                                 destroy_wind_callback_fun_t dwcf, int scale);

  ~zoom_utils_mainwindow();

  void set_window(const fractal_utils::wind_base &w) const noexcept {
    w.copy_to(this->window);
  }

  create_wind_callback_fun_t create_windows_function() const noexcept;
  destroy_wind_callback_fun_t destroy_windows_function() const noexcept;

  void compute_and_paint() noexcept;

  void display_range() noexcept;

private:
  create_wind_callback_fun_t callback_create_wind = nullptr;
  destroy_wind_callback_fun_t callback_destroy_center_wind =
      fractal_utils::callback_destroy_center_wind;

public:
  compute_fractal_callback_fun_t callback_compute_fun = nullptr;
  render_fractal_callback_fun_t callback_render_fun = nullptr;
  export_frame_callback_fun_t callback_export_fun = nullptr;
  hex_encode_fun_t callback_hex_encode_fun =
      fractal_utils::default_hex_encode_fun;
  hex_decode_fun_t callback_hex_decode_fun =
      fractal_utils::default_hex_decode_fun;
  void *custom_parameters = nullptr;
};

#endif // FRACTALUTILS_ZOOM_UTILS_H
