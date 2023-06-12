#ifndef FRACTALUTILS_ZOOMUTILS_ZOOMWIDGET_H
#define FRACTALUTILS_ZOOMUTILS_ZOOMWIDGET_H

#include "core_utils.h"
#include <QMainWindow>
#include <any>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <stack>

namespace fractal_utils {

class zoom_window;
}

namespace Ui {
class zoom_utils_mainwindow;
}

namespace fractal_utils {

struct push_option {
  bool save_map{true};
  bool save_image{true};
  bool save_custom{true};
};

QImage scale_image(const QImage &src, int scale) noexcept;

class zoom_window : public QMainWindow {
  Q_OBJECT

private:
  Ui::zoom_utils_mainwindow *ui;

public:
  explicit zoom_window(QWidget *parent = nullptr);
  virtual ~zoom_window();

  struct compute_result {
    compute_result() = default;
    compute_result(compute_result &&) = default;
    // this function is required by std::stack
    compute_result(const compute_result &);
    compute_result(size_t r, size_t c, size_t fractal_ele_bytes);

    std::unique_ptr<fractal_utils::wind_base> wind{nullptr};
    std::optional<unique_map> fractal{std::nullopt};
    std::optional<QImage> image{std::nullopt};
    std::any custom_data;
  };

private:
  QString m_frame_file_extensions{""};
  std::list<compute_result> m_window_stack;

  push_option push_opt;
  ::fractal_utils::internal::map_base map_base;
  int m_scale{1};

private:
  void compute_current() noexcept;
  void render_current() noexcept;

  void push(compute_result &&) noexcept;

protected:
  virtual std::unique_ptr<fractal_utils::wind_base>
  create_wind() const noexcept = 0;

  virtual std::string encode_hex(const fractal_utils::wind_base &wind_src,
                                 std::string &err) const noexcept;
  virtual void decode_hex(std::string_view hex,
                          std::unique_ptr<wind_base> &wind_unique_ptr,
                          std::string &err) const noexcept;

  virtual void compute(const wind_base &wind, map_view fractal,
                       std::any &custom) const noexcept = 0;
  virtual void render(constant_view fractal, const wind_base &wind,
                      map_view image_u8c3, std::any &custom) const noexcept = 0;
  virtual QString export_frame(QString filename, const wind_base &wind,
                               constant_view fractal, constant_view image_u8c3,
                               std::any &custom) const noexcept;

public:
  push_option push_option() const noexcept { return this->push_opt; }
  void set_push_option(fractal_utils::push_option opt) noexcept {
    this->push_opt = opt;
  }

  const auto &frame_file_extensions() const noexcept {
    return this->m_frame_file_extensions;
  }
  auto &frame_file_extensions() noexcept {
    return this->m_frame_file_extensions;
  }
  void set_frame_file_extensions(const QString &ffes) noexcept {
    this->m_frame_file_extensions = ffes;
  }

  const auto &current_result() const noexcept {
    return this->m_window_stack.back();
  }
  auto &current_result() noexcept { return this->m_window_stack.back(); }

  size_t rows() const noexcept { return this->map_base.rows(); }
  size_t cols() const noexcept { return this->map_base.cols(); }
  size_t fractal_element_bytes() const noexcept {
    return this->map_base.element_bytes();
  }
  inline int scale() const noexcept { return this->m_scale; }
  void set_scale(int s) noexcept {
    this->m_scale = s;
    this->refresh_image_display();
  }

  virtual void reset(size_t r, size_t c, size_t fractal_ele_bytes) noexcept;

  virtual void refresh_image_display() noexcept;
  virtual void refresh_range_display() noexcept;

public slots:

  void received_wheel_move(std::array<int, 2> pos, bool is_scaling_up);
  void received_mouse_move(std::array<int, 2> pos);

  void on_btn_revert_clicked();
  void on_btn_repaint_clicked();
  void on_btn_save_image_clicked();
  void on_btn_save_frame_clicked();
};

} // namespace fractal_utils

#endif // FRACTALUTILS_ZOOMUTILS_ZOOMWIDGET_H