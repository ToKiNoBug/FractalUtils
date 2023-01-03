#include "zoom_utils.h"

#include "ui_zoom_utils_mainwindow.h"

#include "scalable_label.h"

zoom_utils_mainwindow::zoom_utils_mainwindow(
    QWidget *parent, const std::array<int, 2> &window_size)
    : QMainWindow(parent), ui(new Ui::zoom_utils_mainwindow),
      img_u8c3(QSize(window_size[1], window_size[0]),
               QImage::Format::Format_RGB888) {
  ui->setupUi(this);

  connect(this->ui->display, &scalable_label::moved, this,
          &zoom_utils_mainwindow::received_mouse_move);
  connect(this->ui->display, &scalable_label::zoomed, this,
          &zoom_utils_mainwindow::received_wheel_move);

  this->ui->display->resize(window_size[1], window_size[0]);
  {
    QSizePolicy p;
    p.setHorizontalPolicy(QSizePolicy::Fixed);
    p.setVerticalPolicy(QSizePolicy::Fixed);
    this->ui->display->setSizePolicy(p);
  }

  memset(this->img_u8c3.scanLine(0), 0,
         window_size[0] * window_size[1] * sizeof(uint8_t[3]));
  this->ui->display->setPixmap(QPixmap::fromImage(this->img_u8c3));
}

zoom_utils_mainwindow::~zoom_utils_mainwindow() {
  delete ui;

  this->callback_destroy_center_wind(this->window);

  while (!this->previous_windows.empty()) {
    this->callback_destroy_center_wind(this->previous_windows.top());
    this->previous_windows.pop();
  }
}

void fractal_utils::callback_destroy_center_wind(wind_base *const w) {
  delete w;
}

zoom_utils_mainwindow::zoom_utils_mainwindow(
    QWidget *parent, const std::array<int, 2> &window_size,
    create_wind_callback_fun_t cwcf, destroy_wind_callback_fun_t dwcf)
    : zoom_utils_mainwindow(parent, window_size) {
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

int zoom_utils_mainwindow::rows() const noexcept {
  assert(this->img_u8c3.height() == this->map_fractal.rows);
  assert(this->map_fractal.rows == this->ui->display->pixmap().height());
  return (int)this->map_fractal.rows;
}

int zoom_utils_mainwindow::cols() const noexcept {
  assert(this->img_u8c3.width() == this->map_fractal.cols);
  assert(this->map_fractal.cols == this->ui->display->pixmap().width());
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

    auto mm = this->window->displayed_center();
    mm[0] -= this->window->displayed_x_span() / 2;
    mm[1] -= this->window->displayed_y_span() / 2;
    str.push_back('(');
    str.append(QString::number(mm[0]));
    str.append(QStringLiteral(" , "));
    str.append(QString::number(mm[1]));
    str.push_back(')');
    ui->label_minpos->setText(str);
  }

  {
    QString str = QStringLiteral("Maxpos : ");

    auto mm = this->window->displayed_center();
    mm[0] += this->window->displayed_x_span() / 2;
    mm[1] += this->window->displayed_y_span() / 2;
    str.push_back('(');
    str.append(QString::number(mm[0]));
    str.append(QStringLiteral(" , "));
    str.append(QString::number(mm[1]));
    str.push_back(')');
    ui->label_maxpos->setText(str);
  }

  // const double r_span = this->maxmax.fl[1] - this->minmin.fl[1];

  ui->show_scale_y->setText(QString::number(this->window->displayed_y_span()));
  ui->show_scale_x->setText(QString::number(this->window->displayed_x_span()));

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

void zoom_utils_mainwindow::received_mouse_move(std::array<int, 2> pos) {
  std::array<double, 2> coord =
      this->window->displayed_coordinate({this->rows(), this->cols()}, pos);

  QString str = "Mouse : ";
  str.push_back('(');
  str.append(QString::number(coord[0]));
  str.append(QStringLiteral(" , "));
  str.append(QString::number(coord[1]));
  str.push_back(')');

  this->ui->label_mousepos->setText(str);
}

void zoom_utils_mainwindow::received_wheel_move(std::array<int, 2> pos,
                                                bool is_scaling_up) {
  if (!this->lock.try_lock()) {
    return;
  }

  const double ratio = this->ui->spin_zoom_speed->value();

  fractal_utils::wind_base *newwind = this->callback_create_wind();
  this->window->copy_to(newwind);

  this->previous_windows.emplace(newwind);

  this->ui->btn_revert->setDisabled(this->previous_windows.empty());

  this->window->update_center({this->rows(), this->cols()}, pos,
                              (is_scaling_up) ? (ratio) : (1 / ratio));

  this->display_range();

  this->compute_and_paint();

  this->lock.unlock();
}

void zoom_utils_mainwindow::on_btn_revert_clicked() {
  if (!this->lock.try_lock()) {
    return;
  }
  fractal_utils::wind_base *wind = this->previous_windows.top();
  wind->copy_to(this->window);

  this->previous_windows.pop();
  this->ui->btn_revert->setDisabled(this->previous_windows.empty());

  this->display_range();

  this->compute_and_paint();

  this->lock.unlock();
}

void zoom_utils_mainwindow::on_btn_repaint_clicked() {
  if (!this->lock.try_lock()) {
    return;
  }

  fractal_utils::wind_base *new_wind = this->callback_create_wind();

  this->window->copy_to(new_wind);

  this->previous_windows.emplace(new_wind);
  this->ui->btn_revert->setDisabled(this->previous_windows.empty());

  size_t size_of_center_data = 0;
  this->window->center_data(&size_of_center_data);

  QString hex = this->ui->show_center_hex->text();

  if (hex.startsWith("0x") || hex.startsWith("0X")) {
    hex = hex.last(hex.size() - 2);
  }

  // std::string str = hex.toStdString();
  // printf("hex string = %s\n", str.data());

  QByteArray qba = QByteArray::fromHex(hex.toUtf8());

  if (qba.length() != size_of_center_data) {
    printf("\nError : hex have invalid length : should be %i but infact %i\n",
           int(size_of_center_data), (int)qba.length());
    exit(1);
  }

  memcpy(this->window->center_data(), qba.data(), size_of_center_data);

  double new_x_span = this->ui->show_scale_x->text().toDouble();
  double new_y_span = this->ui->show_scale_y->text().toDouble();

  this->window->set_x_span(new_x_span);
  this->window->set_y_span(new_y_span);

  this->display_range();

  this->compute_and_paint();

  this->lock.unlock();
}