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

#include "zoom_window.h"

#include <QFileDialog>
#include <QMessageBox>

#include "scalable_label.h"
#include "ui_zoom_utils_mainwindow.h"

using fractal_utils::zoom_window;

zoom_window::zoom_window(QWidget *parent)
    : QMainWindow(parent), ui{new Ui::zoom_utils_mainwindow} {
  this->ui->setupUi(this);

  connect(this->ui->display, &scalable_label::moved, this,
          &zoom_window::received_mouse_move);
  connect(this->ui->display, &scalable_label::zoomed, this,
          &zoom_window::received_wheel_move);
  {
    QSizePolicy p;
    p.setHorizontalPolicy(QSizePolicy::Expanding);
    p.setVerticalPolicy(QSizePolicy::Expanding);
    this->ui->display->setSizePolicy(p);
  }

  this->setAttribute(Qt::WA_DeleteOnClose, false);
}

zoom_window::~zoom_window() { delete this->ui; }

zoom_window::compute_result::compute_result(const compute_result &src)
//: fractal{src.fractal}, image{src.image}, archive{src.archive}
{
  abort();
  if (src.wind == nullptr) {
    this->wind.reset();
  } else {
    auto wind_p = src.wind->create_another();
    src.wind->copy_to(wind_p);

    this->wind.reset(wind_p);
  }
}

zoom_window::compute_result::compute_result(size_t r, size_t c)
    : image{QImage{QSize{(int)c, (int)r}, QImage::Format::Format_RGB888}} {
  memset(this->image.value().scanLine(0), 0, this->image.value().sizeInBytes());
}

zoom_window::compute_result::compute_result(size_t r, size_t c,
                                            size_t fractal_ele_bytes)
    : image{QImage{QSize{(int)c, (int)r}, QImage::Format::Format_RGB888}} {
  memset(this->image.value().scanLine(0), 0, this->image.value().sizeInBytes());
}

void zoom_window::reset(size_t r, size_t c, size_t fractal_ele_bytes) noexcept {
  while (!this->m_window_stack.empty()) {
    this->m_window_stack.pop();
  }

  this->map_base = {r, c, fractal_ele_bytes};
  {
    compute_result temp{r, c};
    temp.wind = this->create_wind();
    this->m_window_stack.emplace(std::move(temp));
  }

  {
    const auto &windp = this->current_result().wind;
    this->ui->show_scale_x->setText(windp->x_span_string(this->m_ss).c_str());
    this->ui->show_scale_y->setText(windp->y_span_string(this->m_ss).c_str());

    this->refresh_range_display();
  }
  // this->ui->display->setFixedSize(QSize{(int)this->cols(),
  // (int)this->rows()});
  {
    std::string hex, err;
    hex = this->encode_hex(*this->current_result().wind, err);
    if (!err.empty()) {
      return;
    }

    this->ui->show_center_hex->setText(QString::fromLatin1(hex));
  }
}

QImage fractal_utils::scale_image(const QImage &src, int scale) noexcept {
  assert(scale > 0);
  if (scale == 1) {
    return src;
  }

  QSize sz = src.size();
  sz.setHeight(sz.height() * scale);
  sz.setWidth(sz.width() * scale);
  QImage ret(sz, QImage::Format_RGB888);

  memset(ret.scanLine(0), 0, ret.sizeInBytes());

#pragma omp parallel for schedule(static)
  for (int r = 0; r < ret.height(); r++) {
    const fractal_utils::pixel_RGB *const src_data =
        (const fractal_utils::pixel_RGB *)src.scanLine(r / scale);
    fractal_utils::pixel_RGB *const dest_data =
        (fractal_utils::pixel_RGB *)ret.scanLine(r);
    for (int c = 0; c < ret.width(); c++) {
      dest_data[c] = src_data[c / scale];
    }
  }

  return ret;
}

void zoom_window::refresh_image_display() noexcept {
  assert(this->scale() > 0);
  /*
  this->ui->display->setFixedSize(this->cols() * this->scale(),
                                  this->rows() * this->scale());*/
  this->ui->display->setScaledContents(false);

  if (!this->current_result().image.has_value()) {
    this->ui->display->setPixmap(QPixmap{});
  } else {
    this->ui->display->setPixmap(QPixmap::fromImage(
        scale_image(this->current_result().image.value(), this->scale())));
  }
}

void zoom_window::compute_current() noexcept {
  assert(this->m_window_stack.size() > 0);
  auto &top = this->m_window_stack.top();

  this->compute(*top.wind, top.archive);
}

void zoom_window::render_current() noexcept {
  assert(this->m_window_stack.size() > 0);
  auto &top = this->m_window_stack.top();

  const QSize expected_size{(int)this->cols(), (int)this->rows()};
  if (!top.image.has_value() || top.image.value().size() != expected_size) {
    top.image = QImage{expected_size, QImage::Format_RGB888};
  }

  this->render(
      top.archive, *top.wind,
      map_view{top.image.value().scanLine(0), this->rows(), this->cols(), 3});
}

void zoom_window::refresh_range_display() noexcept {
  assert(!this->m_window_stack.empty());

  auto current_wind = this->current_result().wind.get();
  const auto x_span_str = current_wind->x_span_string(this->m_ss);
  const auto y_span_str = current_wind->y_span_string(this->m_ss);
  {
    auto center_str = current_wind->center_string(this->m_ss);
    ui->label_center->setText(
        QStringLiteral("Center: (%1, %2)")
            .arg(center_str[0].c_str(), center_str[1].c_str()));
    ui->show_scale_x->setText(x_span_str.c_str());
    ui->show_scale_y->setText(y_span_str.c_str());
  }
  {
    auto mm = current_wind->right_top_corner_string(this->m_ss);
    this->ui->label_maxpos->setText(
        QStringLiteral("Maxpos: (%1, %2)").arg(mm[0].c_str(), mm[1].c_str()));
  }
  {
    auto mm = current_wind->left_bottom_corner_string(this->m_ss);
    this->ui->label_minpos->setText(
        QStringLiteral("Minpos: (%1, %2)").arg(mm[0].c_str(), mm[1].c_str()));
  }
  {
    std::string err;
    std::string hex = this->encode_hex(*current_wind, err);

    if (!err.empty()) {
      QMessageBox::critical(
          this, "Failed to encode binary to hex string.",
          QStringLiteral("this->callback_hex_encode_fun failed. \nDetail: %1\n")
              .arg(QString::fromUtf8(err.data())));

      abort();
    }
    ui->show_center_hex->setText(QString::fromLatin1(hex.c_str()));
  }
}

void zoom_window::received_mouse_move(std::array<int, 2> pos) {
  pos[0] /= this->scale();
  pos[1] /= this->scale();
  const auto coord = this->current_result().wind->coordinate_string(
      {(int)this->rows(), (int)this->cols()}, pos, this->m_ss);

  this->ui->label_mousepos->setText(
      QStringLiteral("Mouse : (%1, %2)")
          .arg(coord[0].c_str(), coord[1].c_str()));
}

void zoom_window::push(compute_result &&new_res) noexcept {
  auto &old = this->current_result();

  if (!this->push_opt.save_archive) {
    old.archive.reset();
  }
  if (!this->push_opt.save_image) {
    old.image.reset();
  }

  // #warning here
  this->m_window_stack.emplace(std::move(new_res));
}

void zoom_window::received_wheel_move(std::array<int, 2> pos,
                                      bool is_scaling_up) {
  /*if (!this->lock.try_lock()) {
    return;
  }
*/
  pos[0] /= this->scale();
  pos[1] /= this->scale();

  const double ratio = this->ui->spin_zoom_speed->value();

  auto &old = this->current_result();

  {
    compute_result res{this->rows(), this->cols()};
    res.wind = this->create_wind();
    old.wind->copy_to(res.wind.get());

    res.archive = old.archive;

    res.wind->update_center({(int)this->rows(), (int)this->cols()}, pos,
                            (is_scaling_up) ? (ratio) : (1 / ratio));

    this->push(std::move(res));
  }

  this->ui->btn_revert->setDisabled(this->m_window_stack.size() <= 1);

  this->refresh_range_display();

  this->compute_current();
  this->render_current();
  this->refresh_image_display();

  // this->lock.unlock();
}

void zoom_window::on_btn_revert_clicked() {
  if (this->m_window_stack.size() <= 1) {
    return;
  }

  this->m_window_stack.pop();

  this->ui->btn_revert->setDisabled(this->m_window_stack.size() <= 1);
  this->refresh_range_display();
  auto &cur = this->current_result();
  if (!cur.archive.has_value()) {
    this->compute_current();
  }

  if (!cur.image.has_value()) {
    this->render_current();
  }

  this->refresh_image_display();
}

void zoom_window::on_btn_repaint_clicked() {
  auto &old = this->current_result();

  {
    // get current wind
    auto current_wind = this->create_wind();
    old.wind->copy_to(current_wind.get());
    {
      std::string current_hex =
          this->ui->show_center_hex->text().toLatin1().data();
      std::string err;
      this->decode_hex(current_hex, current_wind, err);
      if (!err.empty()) {
        QMessageBox::critical(
            this, "Invalid hex string",
            QStringLiteral("this->callback_hex_decode_fun failed to decode "
                           "hex string \"%1\" to binary. \nDetail: \n%2")
                .arg(QString::fromUtf8(current_hex),
                     QString::fromUtf8(err.data())));
        // this->lock.unlock();
        return;
      }

      const auto new_x_span = this->ui->show_scale_x->text().toLatin1();
      const auto new_y_span = this->ui->show_scale_y->text().toLatin1();

      if (!current_wind->set_x_span(new_x_span.data(), this->m_ss)) {
        QMessageBox::critical(
            this, "Invalid value for x span",
            QStringLiteral("%1 can not be converted to an floating point type.")
                .arg(new_x_span.data()));
        return;
      }
      if (!current_wind->set_y_span(new_y_span.data(), this->m_ss)) {
        QMessageBox::critical(
            this, "Invalid value for y span",
            QStringLiteral("%1 can not be converted to an floating point type.")
                .arg(new_x_span.data()));
        return;
      }
    }

    if (*current_wind != *old.wind) {
      compute_result res{this->rows(), this->cols()};
      res.wind = std::move(current_wind);
      res.archive = old.archive;
      this->push(std::move(res));
    } else {
      // do not push, compute the current frame again
    }
  }

  this->refresh_range_display();

  this->compute_current();
  this->render_current();
  this->refresh_image_display();
}

void zoom_window::on_btn_save_image_clicked() {
  if (this->m_window_stack.empty() ||
      !this->current_result().image.has_value()) {
    QMessageBox::warning(this, "Can not save as image",
                         "No image to be displayed.");
    return;
  }

  static QString prev_dir{""};
  const QString path = QFileDialog::getSaveFileName(this, "Save current image",
                                                    "", "*.png;;*.jpg;;*.gif");
  if (path.isEmpty()) {
    return;
  }
  prev_dir = QFileInfo{path}.dir().absolutePath();

  if (!this->current_result().image.value().save(path)) {
    QMessageBox::warning(this, "Failed to save image",
                         QStringLiteral("Failed to generate %1").arg(path));
  }
}

QString zoom_window::export_frame(QString filename, const wind_base &wind,

                                  constant_view image_u8c3,
                                  std::any &archive) const noexcept {
  return "Can not export the frame because virtual function named "
         "\"export_frame\" is not overrided. This reply is from the default "
         "implementation of zoom_window.";
}

void zoom_window::on_btn_save_frame_clicked() {
  if (this->m_frame_file_extensions.isEmpty()) {
    return;
  }

  auto &cur = this->current_result();

  if (!cur.archive.has_value()) {
    QMessageBox::warning(this, "No data to export",
                         "fractal.has_value()==false");
    return;
  }

  static QString prev_dir{""};
  QString path = QFileDialog::getSaveFileName(
      this, "Save current frame to custom file format", "",
      this->m_frame_file_extensions);

  if (path.isEmpty()) {
    return;
  }
  prev_dir = QFileInfo{path}.dir().absolutePath();

  QString err;
  if (cur.image.has_value()) {
    const auto &img = cur.image.value();
    err =
        this->export_frame(path, *cur.wind,
                           constant_view{img.scanLine(0), (size_t)img.height(),
                                         (size_t)img.width(), 3},
                           cur.archive);
  } else {
    err = this->export_frame(path, *cur.wind, constant_view{nullptr, 0, 0, 3},
                             cur.archive);
  }

  if (!err.isEmpty()) {
    QMessageBox::warning(this, "Failed to export custom frame.",
                         QStringLiteral("Details: %1").arg(err));
    return;
  }
  return;
}

#include "hex_convert.h"

std::string zoom_window::encode_hex(const fractal_utils::wind_base &wind_src,
                                    std::string &err) const noexcept {
  err.clear();

  size_t len = 0;
  const void *const src = wind_src.center_data(&len);

  std::string ret;
  ret.resize(len * 32);

  auto strlen_opt =
      fractal_utils::bin_2_hex(src, len, ret.data(), ret.size(), true);

  assert(strlen_opt.has_value());

  ret.resize(strlen_opt.value());
  return ret;
}

void zoom_window::decode_hex(
    std::string_view hex, std::unique_ptr<fractal_utils::wind_base> &wind_dest,
    std::string &err) const noexcept {
  err.clear();
  size_t dst_bytes = 0;
  void *const dst = wind_dest->center_data(&dst_bytes);

  auto bytes = fractal_utils::hex_2_bin(hex, dst, dst_bytes);

  if (!bytes.has_value()) {
    err = "Failed to convert hex to binary, the hex string may be invalid.";
    return;
  }

  if (bytes.value() != dst_bytes) {
    err = "The length of hex is invalid. Expected " +
          std::to_string(dst_bytes) + " bytes, but actually " +
          std::to_string(bytes.value()) + " bytes written.";
  }
}

void zoom_window::set_label_widget(scalable_label *label) & noexcept {
  auto old_label = this->ui->display;
  label->setPixmap(old_label->pixmap());
  label->setSizePolicy(old_label->sizePolicy());

  this->ui->display->deleteLater();
  this->ui->display = label;
  label->setParent(this);

  this->ui->gl_display_placer->addWidget(label, 0, 0);

  this->refresh_image_display();
}
[[nodiscard]] scalable_label *zoom_window::label_widget() noexcept {
  return this->ui->display;
}
[[nodiscard]] const scalable_label *zoom_window::label_widget() const noexcept {
  return this->ui->display;
}