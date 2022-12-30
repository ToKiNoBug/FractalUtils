#include "zoom_utils.h"

#include "ui_zoom_utils_mainwindow.h"

zoom_utils_mainwindow::zoom_utils_mainwindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::zoom_utils_mainwindow) {
  ui->setupUi(this);
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
  this->callback_compute_fun(*this->window, &this->map_fractal);
}