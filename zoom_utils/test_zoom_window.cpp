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
#include <QApplication>
#include <complex>

using fractal_utils::zoom_window;

struct compute_option {
  int16_t maxit{500};
  // fractal_utils::unique_map float_map;
};

struct archive {
  fractal_utils::unique_map mat;
  compute_option option;
};

int16_t iterate(std::complex<double> C, int16_t maxit) noexcept {
  int16_t i = 0;
  std::complex<double> z = C;
  while (true) {
    if (i > maxit) return -1;

    if (z.real() * z.real() + z.imag() * z.imag() >= 4) {
      break;
    }
    z = z * z + C;
    i++;
  }
  return i;
}

class window : public zoom_window {
  // Q_OBJECT
 private:
 public:
  explicit window(QWidget *parent = nullptr) : zoom_window{parent} {}

  std::unique_ptr<fractal_utils::unique_map> float_map;

 protected:
  std::unique_ptr<fractal_utils::wind_base> create_wind()
      const noexcept override {
    std::unique_ptr<fractal_utils::wind_base> ret;
    ret.reset(new fractal_utils::center_wind<double>);
    return ret;
  }

  void compute(const fractal_utils::wind_base &_wind,
               std::any &archive) const noexcept override {
    assert(_wind.float_type_matches<double>());

    const auto &wind =
        dynamic_cast<const fractal_utils::center_wind<double> &>(_wind);

    auto &ar = std::any_cast<::archive &>(archive);
    ar.mat.reset(this->rows(), this->cols(), sizeof(uint16_t));

    const std::complex<double> lt(wind.left_top_corner()[0],
                                  wind.left_top_corner()[1]);
    const std::complex<double> rb(wind.right_bottom_corner()[0],
                                  wind.right_bottom_corner()[1]);
    std::complex<double> scale = rb - lt;

    scale = std::complex<double>(scale.real() / ar.mat.cols(),
                                 scale.imag() / ar.mat.rows());

#pragma omp parallel for schedule(dynamic)
    for (int r = 0; r < ar.mat.rows(); r++) {
      for (int c = 0; c < ar.mat.cols(); c++) {
        std::complex<double> C(scale.real() * c, scale.imag() * r);
        C += lt;

        ar.mat.at<int16_t>(r, c) = iterate(C, ar.option.maxit);
      }
    }
  }

  void render(std::any &archive, const fractal_utils::wind_base &wind,
              fractal_utils::map_view image_u8c3) const noexcept override {
    auto &ar = std::any_cast<::archive &>(archive);
    assert(ar.mat.rows() == this->rows());
    assert(ar.mat.cols() == this->cols());
    assert(ar.mat.element_bytes() == sizeof(uint16_t));

    assert(ar.mat.rows() == image_u8c3.rows());
    assert(ar.mat.cols() == image_u8c3.cols());
    assert(ar.mat.rows() == this->float_map->rows());
    assert(ar.mat.cols() == this->float_map->cols());

#pragma omp parallel for schedule(dynamic)
    for (int r = 0; r < ar.mat.rows(); r++) {
      for (int c = 0; c < ar.mat.cols(); c++) {
        if (ar.mat.at<int16_t>(r, c) < 0) {
          this->float_map->at<float>(r, c) = 0;
        } else {
          this->float_map->at<float>(r, c) =
              float(ar.mat.at<int16_t>(r, c)) / ar.option.maxit;
        }
      }

      fractal_utils::color_u8c3_many(
          this->float_map->address<float>(r, 0),
          fractal_utils::color_series::parula, ar.mat.cols(),
          image_u8c3.address<fractal_utils::pixel_RGB>(r, 0));

      for (int c = 0; c < ar.mat.cols(); c++) {
        if (ar.mat.at<int16_t>(r, c) < 0) {
          image_u8c3.at<fractal_utils::pixel_RGB>(r, c) = {0, 0, 0};
        }
      }
    }
  }
};

int main(int argc, char **argv) {
  QApplication qapp{argc, argv};

  window wind{};
  {
    QLocale locale;
    if (locale.language() == QLocale::Language::Chinese) {
      wind.set_language(fractal_utils::language_t::zh_CN);
    }
  }

  const int rows = 720;
  const int cols = 1080;
  wind.reset(rows, cols);
  assert(wind.rows() == rows);
  assert(wind.cols() == cols);
  wind.set_scale(1);

  wind.float_map.reset(new fractal_utils::unique_map);
  wind.float_map->reset(rows, cols, sizeof(float));

  {
    archive ar;
    ar.option.maxit = 500;
    wind.current_result().archive = std::move(ar);
  }
  {
    auto &cwindp = wind.current_result().wind;
    cwindp->set_y_span(4);
    cwindp->set_x_span(6);

    dynamic_cast<fractal_utils::center_wind<double> *>(cwindp.get())->center = {
        0, 0};
    wind.refresh_range_display();
  }

  wind.show();

  // delete wind;

  return qapp.exec();
}