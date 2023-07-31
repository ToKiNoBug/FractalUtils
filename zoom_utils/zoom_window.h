/*
Copyright © 2022-2023  TokiNoBug
This file is part of FractalUtils.

FractalUtils is free software: you can redistribute it and/or modify
                                                                    it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

                                        FractalUtils is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with FractalUtils.  If not, see <https://www.gnu.org/licenses/>.

   Contact with me:
   github:https://github.com/ToKiNoBug
*/

#ifndef FRACTALUTILS_ZOOMUTILS_ZOOMWIDGET_H
#define FRACTALUTILS_ZOOMUTILS_ZOOMWIDGET_H

#include "core_utils.h"
#include <QMainWindow>
#include <any>
#include <memory>
#include <mutex>
#include <optional>
#include <stack>
#include <QTranslator>

#include "scalable_label.h"

namespace fractal_utils {

class zoom_window;
}

namespace Ui {
class zoom_utils_mainwindow;
}

namespace fractal_utils {

struct push_options {
  bool save_archive{true};
  bool save_image{true};
};

enum class language_t { en_US, zh_CN };

QImage scale_image(const QImage &src, int scale) noexcept;

class zoom_window : public QMainWindow {
  Q_OBJECT
 protected:
  Ui::zoom_utils_mainwindow *ui;

 public:
  explicit zoom_window(QWidget *parent = nullptr);
  ~zoom_window() override;

  struct compute_result {
    compute_result() = default;
    compute_result(compute_result &&) = default;
    // this function is required by std::stack but never called
    [[noreturn]] compute_result(const compute_result &);
    compute_result(size_t r, size_t c);

    [[deprecated(
        "fractal_ele_bytes is no longer used, use compute_result(size_t r, "
        "size_t c) instead!")]] compute_result(size_t r, size_t c,
                                               size_t fractal_ele_bytes);

    std::unique_ptr<fractal_utils::wind_base> wind{nullptr};
    std::any archive{};
    std::optional<QImage> image{std::nullopt};
  };

 protected:
  QString m_frame_file_extensions{""};
  std::stack<compute_result> m_window_stack;

  push_options push_opt;
  size_t m_rows;
  size_t m_cols;
  int m_scale{1};
  std::stringstream m_ss;

  QWidget *m_custom_widget{nullptr};

  language_t m_current_lang{language_t::en_US};
  QTranslator m_translator_zoom_window;

 protected:
  virtual void compute_current() & noexcept;
  virtual void render_current() & noexcept;

  virtual void push(compute_result &&) & noexcept;

 protected:
  [[nodiscard]] virtual std::unique_ptr<fractal_utils::wind_base> create_wind()
      const noexcept = 0;

  virtual std::string encode_hex(const fractal_utils::wind_base &wind_src,
                                 std::string &err) const noexcept;
  virtual void decode_hex(std::string_view hex,
                          std::unique_ptr<wind_base> &wind_unique_ptr,
                          std::string &err) const noexcept;

  virtual void compute(const wind_base &wind,
                       std::any &archive) const noexcept = 0;
  virtual void render(std::any &archive, const wind_base &wind,
                      map_view image_u8c3) const noexcept = 0;
  virtual QString export_frame(QString filename, const wind_base &wind,
                               constant_view image_u8c3,
                               std::any &custom) const noexcept;

 public:
  [[nodiscard]] auto current_language() const noexcept {
    return this->m_current_lang;
  }

  virtual QString set_language(language_t lang) & noexcept;

  [[nodiscard]] push_options push_option() const noexcept {
    return this->push_opt;
  }
  void set_push_option(fractal_utils::push_options opt) & noexcept {
    this->push_opt = opt;
  }

  [[nodiscard]] const auto &frame_file_extensions() const noexcept {
    return this->m_frame_file_extensions;
  }
  auto &frame_file_extensions() noexcept {
    return this->m_frame_file_extensions;
  }
  void set_frame_file_extensions(const QString &ffes) & noexcept {
    this->m_frame_file_extensions = ffes;
  }

  [[nodiscard]] const auto &current_result() const noexcept {
    return this->m_window_stack.top();
  }
  auto &current_result() noexcept { return this->m_window_stack.top(); }

  [[nodiscard]] inline size_t rows() const noexcept { return this->m_rows; }
  [[nodiscard]] inline size_t cols() const noexcept { return this->m_cols; }

  [[nodiscard]] inline int scale() const noexcept { return this->m_scale; }
  virtual void set_scale(int s) noexcept {
    this->m_scale = s;
    this->refresh_image_display();
  }

  virtual void reset(size_t r, size_t c) & noexcept;

  virtual void refresh_image_display() & noexcept;
  virtual void refresh_range_display() & noexcept;

  virtual void set_label_widget(scalable_label *label) & noexcept;
  [[nodiscard]] scalable_label *label_widget() noexcept;
  [[nodiscard]] const scalable_label *label_widget() const noexcept;

  virtual void set_custom_widget(QWidget *widget) & noexcept;
  [[nodiscard]] QWidget *custom_widget() noexcept;
  [[nodiscard]] const QWidget *custom_widget() const noexcept;

 public slots:

  virtual void received_wheel_move(std::array<int, 2> pos, bool is_scaling_up);
  virtual void received_mouse_move(std::array<int, 2> pos);

  virtual void on_btn_revert_clicked();
  virtual void on_btn_repaint_clicked();
  virtual void on_btn_save_image_clicked();
  virtual void on_btn_save_frame_clicked();
};

}  // namespace fractal_utils

#endif  // FRACTALUTILS_ZOOMUTILS_ZOOMWIDGET_H