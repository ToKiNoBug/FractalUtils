#include "colors.h"

#include <stdio.h>

int main() {
  const uint32_t val =
      fractal_utils::color_u8c4(0.9, fractal_utils::color_series::pink);

  printf("val = 0x%X\n", val);

  return 0;
}