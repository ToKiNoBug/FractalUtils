#include "zoom_utils.h"

#include "ui_zoom_utils_mainwindow.h"

#include "scalable_label.h"

zoom_utils_mainwindow::zoom_utils_mainwindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::zoom_utils_mainwindow) {
  ui->setupUi(this);

  connect(this->ui->display, &scalable_label::moved, this,
          &zoom_utils_mainwindow::received_mouse_move);
  connect(this->ui->display, &scalable_label::zoomed, this,
          &zoom_utils_mainwindow::received_wheel_move);
}

zoom_utils_mainwindow::~zoom_utils_mainwindow() {
  delete ui;

  this->callback_destroy_center_wind(this->window);
}

void fractal_utils::callback_destroy_center_wind(wind_base *const w) {
  delete w;
}

zoom_utils_mainwindow::zoom_utils_mainwindow(
    QWidget *parent, const std::array<int, 2> &window_size,
    create_wind_callback_fun_t cwcf, destroy_wind_callback_fun_t dwcf)
    : zoom_utils_mainwindow(parent) {
  this->callback_create_wind = cwcf;
  this->callback_destroy_center_wind = dwcf;

  this->window = this->callback_create_wind();

  this->ui->display->resize(window_size[1], window_size[0]);
  {
    QSizePolicy p;
    p.setHorizontalPolicy(QSizePolicy::Fixed);
    p.setVerticalPolicy(QSizePolicy::Fixed);
    this->ui->display->setSizePolicy(p);
  }

  this->img_u8c3 = QImage(QSize(window_size[0], window_size[0]),
                          QImage::Format::Format_RGB888);
  memset(this->img_u8c3.scanLine(0), 255,
         window_size[0] * window_size[1] * sizeof(uint8_t[3]));
  this->ui->display->setPixmap(QPixmap::fromImage(this->img_u8c3));
}

zoom_utils_mainwindow::create_wind_callback_fun_t
zoom_utils_mainwindow::create_windows_function() const noexcept {
  return this->callback_create_wind;
}

zoom_utils_mainwindow::destroy_wind_callback_fun_t
zoom_utils_mainwindow::destroy_windows_function() const noexcept {
  return this->callback_destroy_center_wind;
}

int zoom_utils_mainwindow::rows() const noexcept {
  assert(this->img_u8c3.rows() == this->map_fractal.rows);
  assert(this->map_fractal.rows == this->ui->display.image().rows());
  return (int)this->map_fractal.rows;
}

int zoom_utils_mainwindow::cols() const noexcept {
  assert(this->img_u8c3.cols() == this->map_fractal.cols);
  assert(this->map_fractal.cols == this->ui->display.image().cols());
  return (int)this->map_fractal.cols;
}

void zoom_utils_mainwindow::compute_and_paint() noexcept {
  this->callback_compute_fun(*this->window, this->custom_parameters,
                             &this->map_fractal);
  fractal_utils::fractal_map map;
  map.rows = this->img_u8c3.height();
  map.cols = this->img_u8c3.width();
  map.element_bytes = 3;
  map.data = this->img_u8c3.scanLine(0);
  this->callback_render_fun(this->map_fractal, *this->window,
                            this->custom_parameters, &map);

  this->ui->display->setPixmap(QPixmap::fromImage(this->img_u8c3));
}

void zoom_utils_mainwindow::display_range() noexcept {

  {
    QString str = QStringLiteral("Minpos : ");

    auto mm = this->window->displayed_coordinate({this->rows(), this->cols()},
                                                 {this->rows() - 1, 0});
    str.push_back('(');
    str.append(QString::number(mm[0]));
    str.append(QStringLiteral(" , "));
    str.append(QString::number(mm[1]));
    str.push_back(')');
    ui->label_minpos->setText(str);
  }

  {
    QString str = QStringLiteral("Maxpos : ");

    auto mm = this->window->displayed_coordinate({this->rows(), this->cols()},
                                                 {0, this->cols() - 1});
    str.push_back('(');
    str.append(QString::number(mm[0]));
    str.append(QStringLiteral(" , "));
    str.append(QString::number(mm[1]));
    str.push_back(')');
    ui->label_maxpos->setText(str);
  }

  // const double r_span = this->maxmax.fl[1] - this->minmin.fl[1];

  ui->show_scale->setText(QString::number(this->window->displayed_y_span()));

  auto center = this->window->displayed_center();

  {
    QString str = QStringLiteral("Center : ");
    str.push_back('(');
    str.append(QString::number(center[0]));
    str.append(QStringLiteral(" , "));
    str.append(QString::number(center[1]));
    str.push_back(')');

    ui->label_center->setText(str);
  }

  {
    QString str = QStringLiteral("0x");
    size_t bytes = 0;
    void *bin = this->window->center_data(&bytes);

    str +=
        QByteArray::fromRawData(reinterpret_cast<char *>(bin), bytes).toHex();

    ui->show_center_hex->setText(str);
  }
}

void zoom_utils_mainwindow::received_mouse_move(std::array<int, 2> pos) {}

void zoom_utils_mainwindow::received_wheel_move(std::array<int, 2> pos,
                                                bool is_scaling_up) {}