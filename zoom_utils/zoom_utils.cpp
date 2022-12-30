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
}

zoom_utils_mainwindow::create_wind_callback_fun_t
zoom_utils_mainwindow::create_windows_function() const noexcept {
  return this->callback_create_wind;
}

zoom_utils_mainwindow::destroy_wind_callback_fun_t
zoom_utils_mainwindow::destroy_windows_function() const noexcept {
  return this->callback_destroy_center_wind;
}