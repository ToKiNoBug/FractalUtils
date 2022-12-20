#include "fractal_png.h"

#include "../core_utils/colors.h"

#include <cstring>
#include <stdio.h>

using namespace fractal_utils;

int main() {

  // printf("rua~\n");

  fractal_map map = fractal_map::create(128, 256, 3);

  // printf("address = %p\n", map.data);

  memset(map.data, 0xAF, map.cols * map.rows * map.element_bytes);

  bool success = true;

  if (map.element_bytes == 3) {

    pixel_RGB *const data = reinterpret_cast<pixel_RGB *>(map.data);

    for (uint64_t i = 0; i < map.rows * map.cols; i++) {
      data[i].value[i % 3] = 0xFF;
    }

    success = write_png("test_u8c3.png", color_space::u8c3, map);
  }

  printf("success = %i.\n", int(success));

  return 0;
}