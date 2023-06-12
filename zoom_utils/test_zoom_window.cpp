#include "zoom_window.h"
#include <QApplication>
#include <complex>

using fractal_utils::zoom_window;

struct compute_option {
  int16_t maxit{500};
  // fractal_utils::unique_map float_map;
};

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

class window : public zoom_window {
  // Q_OBJECT
private:
public:
  explicit window(QWidget *parent = nullptr) : zoom_window{parent} {}

  std::unique_ptr<fractal_utils::unique_map> float_map;

protected:
  std::unique_ptr<fractal_utils::wind_base>
  create_wind() const noexcept override {
    std::unique_ptr<fractal_utils::wind_base> ret;
    ret.reset(new fractal_utils::center_wind<double>);
    return ret;
  }

  void compute(const fractal_utils::wind_base &_wind,
               fractal_utils::map_view fractal,
               std::any &custom) const noexcept override {
    assert(_wind.float_type_matches<double>());

    assert(fractal.rows() == this->rows());
    assert(fractal.cols() == this->cols());
    assert(fractal.element_bytes() == this->fractal_element_bytes());

    const auto &wind =
        dynamic_cast<const fractal_utils::center_wind<double> &>(_wind);

    const auto &option = std::any_cast<compute_option>(custom);

    const std::complex<double> lt(wind.left_top_corner()[0],
                                  wind.left_top_corner()[1]);
    const std::complex<double> rb(wind.right_bottom_corner()[0],
                                  wind.right_bottom_corner()[1]);
    std::complex<double> scale = rb - lt;

    scale = std::complex<double>(scale.real() / fractal.cols(),
                                 scale.imag() / fractal.rows());

#pragma omp parallel for schedule(dynamic)
    for (int r = 0; r < fractal.rows(); r++) {
      for (int c = 0; c < fractal.cols(); c++) {
        std::complex<double> C(scale.real() * c, scale.imag() * r);
        C += lt;

        fractal.at<int16_t>(r, c) = iterate(C, option.maxit);
      }
    }
  }

  void render(fractal_utils::constant_view fractal,
              const fractal_utils::wind_base &wind,
              fractal_utils::map_view image_u8c3,
              std::any &custom) const noexcept override {
    assert(fractal.rows() == this->rows());
    assert(fractal.cols() == this->cols());
    assert(fractal.element_bytes() == this->fractal_element_bytes());

    assert(fractal.rows() == image_u8c3.rows());
    assert(fractal.cols() == image_u8c3.cols());
    assert(fractal.rows() == this->float_map->rows());
    assert(fractal.cols() == this->float_map->cols());

    auto &option = std::any_cast<compute_option &>(custom);

#pragma omp parallel for schedule(dynamic)
    for (int r = 0; r < fractal.rows(); r++) {
      for (int c = 0; c < fractal.cols(); c++) {
        if (fractal.at<int16_t>(r, c) < 0) {
          this->float_map->at<float>(r, c) = 0;
        } else {
          this->float_map->at<float>(r, c) =
              float(fractal.at<int16_t>(r, c)) / option.maxit;
        }
      }

      fractal_utils::color_u8c3_many(
          this->float_map->address<float>(r, 0),
          fractal_utils::color_series::parula, fractal.cols(),
          image_u8c3.address<fractal_utils::pixel_RGB>(r, 0));

      for (int c = 0; c < fractal.cols(); c++) {
        if (fractal.at<int16_t>(r, c) < 0) {
          image_u8c3.at<fractal_utils::pixel_RGB>(r, c) = {0, 0, 0};
        }
      }
    }
  }
};

int main(int argc, char **argv) {
  QApplication qapp{argc, argv};
  //{ window windA; }

  window wind{};
  const int rows = 720;
  const int cols = 1080;
  wind.reset(rows, cols, sizeof(int16_t));
  assert(wind.rows() == rows);
  assert(wind.cols() == cols);
  wind.set_scale(1);

  wind.float_map.reset(new fractal_utils::unique_map);
  wind.float_map->reset(rows, cols, sizeof(float));

  {
    compute_option opt;
    opt.maxit = 500;

    wind.current_result().custom_data = std::move(opt);
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