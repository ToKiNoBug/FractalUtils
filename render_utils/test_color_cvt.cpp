#include <array>
#include <render_utils.h>
#include <span>
#include <cmath>
#include <fmt/format.h>
#include <color_cvt.hpp>

struct color_pair {
  std::array<float, 3> rgb;
  std::array<float, 3> hsv;
};

bool check(std::span<const float, 3> a, std::span<const float, 3> b) noexcept {
  constexpr float epsilon = 5e-3f;
  for (size_t idx = 0; idx < 3; idx++) {
    float err = a[idx] - b[idx];
    if (std::abs(err) >= epsilon) {
      return false;
    }
  }
  return true;
}

int main() {
  const std::array<color_pair, 7> color_list{
      color_pair{.rgb{1, 0.071, 0}, .hsv{4.2 + 360, 1, 1}},
      color_pair{.rgb{0.75, 0.745, 0.429}, .hsv{59.1 - 360, 0.429, 0.75}},
      color_pair{.rgb{0.75, 0.839, 0.742}, .hsv{115.3, 0.116, 0.839}},
      color_pair{.rgb{0.045, 0.5, 0.468}, .hsv{175.8, 0.911, 0.5}},
      color_pair{.rgb{0.079, 0.079, 0.089},
                 .hsv{240.5 - 360 * 9, 0.116, 0.089}},
      color_pair{.rgb{0.482, 0.202, 0.491}, .hsv{298.1, 0.589, 0.491}},
      color_pair{.rgb{1, 0, 0.094}, .hsv{354.4 + 360 * 9, 1, 1}}};

  int err_counter = 0;

  for (auto& cp : color_list) {
    std::array<float, 3> rgb_computed;
    fractal_utils::hsv_to_rgb(cp.hsv[0], cp.hsv[1], cp.hsv[2], rgb_computed[0],
                              rgb_computed[1], rgb_computed[2]);

    if (!check(cp.rgb, rgb_computed)) {
      fmt::print(
          "Color conversion failed with hsv = [{},{},{}], expected rgb = "
          "[{},{},{}], but the result is [{},{},{}]\n",
          cp.hsv[0], cp.hsv[1], cp.hsv[2], cp.rgb[0], cp.rgb[1], cp.rgb[2],
          rgb_computed[0], rgb_computed[1], rgb_computed[2]);
      err_counter++;
    }
  }

  return err_counter;
}