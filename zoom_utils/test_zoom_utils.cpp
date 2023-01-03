#include "zoom_utils.h"

#include <QApplication>

#include <complex>

#include <omp.h>

void callback_compute(const fractal_utils::wind_base &wind, void *custom_ptr,
                      fractal_utils::fractal_map *map_fractal);

void callback_render(const fractal_utils::fractal_map &map_fractal,
                     const fractal_utils::wind_base &window, void *custom_ptr,
                     fractal_utils::fractal_map *map_u8c3_do_not_resize);

struct compute_opt {
  int16_t max_it;
  fractal_utils::fractal_map fltmap;
};

int main(int argC, char **argV) {
  QApplication qga(argC, argV);
  using namespace fractal_utils;

  omp_set_num_threads(20);
  const int resolution_rows = 640;
  const int resolution_cols = 640;

  mainwindow w(double(1), nullptr, {resolution_rows, resolution_cols});

  w.show();
  {
    fractal_utils::center_wind<double> __w;
    __w.center = {0, 0};
    __w.y_span = 4;
    __w.x_span = __w.y_span / resolution_rows * resolution_cols;
    w.set_window(__w);
  }
  w.map_fractal = fractal_utils::fractal_map::create(
      resolution_rows, resolution_cols, sizeof(int16_t));

  w.callback_compute_fun = callback_compute;
  w.callback_render_fun = callback_render;
  compute_opt opt;
  opt.max_it = 10000;
  opt.fltmap = fractal_utils::fractal_map::create(
      resolution_rows, resolution_cols, sizeof(float));
  w.custom_parameters = &opt;

  // printf("size = %i, %i\n", w.rows(), w.cols());

  w.display_range();

  w.compute_and_paint();

  return qga.exec();
}

int16_t iterate(std::complex<double> C, int16_t maxit) noexcept {
  int16_t i = 0;
  std::complex<double> z = C;
  while (true) {
    if (i > maxit)
      return -1;

    if (z.real() * z.real() + z.imag() * z.imag() >= 4) {
      break;
    }
    z = z * z + C;
    i++;
  }
  return i;
}

void callback_compute(const fractal_utils::wind_base &__wind, void *custom_ptr,
                      fractal_utils::fractal_map *map_fractal) {
  const fractal_utils::center_wind<double> &wind =
      dynamic_cast<const fractal_utils::center_wind<double> &>(__wind);

  const compute_opt *opt = (const compute_opt *)custom_ptr;

  const std::complex<double> lt(wind.left_top_corner()[0],
                                wind.left_top_corner()[1]);
  const std::complex<double> rb(wind.right_bottom_corner()[0],
                                wind.right_bottom_corner()[1]);
  std::complex<double> scale = rb - lt;
  // printf("lt = %F + %Fi\n", lt.real(), lt.imag());
  // printf("rb = %F + %Fi\n", rb.real(), rb.imag());
  // printf("scale = %F + %Fi\n", scale.real(), scale.imag());
  scale = std::complex<double>(scale.real() / map_fractal->cols,
                               scale.imag() / map_fractal->rows);

#pragma omp parallel for schedule(dynamic)
  for (int r = 0; r < map_fractal->rows; r++) {
    for (int c = 0; c < map_fractal->cols; c++) {
      std::complex<double> C(scale.real() * c, scale.imag() * r);
      C += lt;

      map_fractal->at<int16_t>(r, c) = iterate(C, opt->max_it);
    }
  }
  /*
  printf("%i Computations finished.\n",
         int(map_fractal->rows * map_fractal->cols));
         */
}

void callback_render(const fractal_utils::fractal_map &map_fractal,
                     const fractal_utils::wind_base &window, void *custom_ptr,
                     fractal_utils::fractal_map *map_u8c3_do_not_resize) {

  assert(map_fractal.rows == map_u8c3_do_not_resize->rows);

  assert(map_fractal.cols == map_u8c3_do_not_resize->cols);

  compute_opt &opt = *(compute_opt *)custom_ptr;

  assert(map_fractal.rows == opt.fltmap.rows);
  assert(map_fractal.cols == opt.fltmap.cols);

  assert(map_u8c3_do_not_resize->element_bytes == 3);

#pragma omp parallel for schedule(dynamic)
  for (int r = 0; r < map_fractal.rows; r++) {
    for (int c = 0; c < map_fractal.cols; c++) {
      if (map_fractal.at<int16_t>(r, c) < 0) {
        opt.fltmap.at<float>(r, c) = 0;
      } else {
        opt.fltmap.at<float>(r, c) =
            float(map_fractal.at<int16_t>(r, c)) / opt.max_it;
      }
    }

    fractal_utils::color_u8c3_many(
        &opt.fltmap.at<float>(r, 0), fractal_utils::color_series::jet,
        map_fractal.cols,
        &map_u8c3_do_not_resize->at<fractal_utils::pixel_RGB>(r, 0));

    for (int c = 0; c < map_fractal.cols; c++) {
      if (map_fractal.at<int16_t>(r, c) < 0) {
        map_u8c3_do_not_resize->at<fractal_utils::pixel_RGB>(r, c) = {0, 0, 0};
      }
    }
  }

  // printf("Render finished.\n");
}
