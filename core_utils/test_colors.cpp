/*
 Copyright © 2022-2023  TokiNoBug
This file is part of FractalUtils.

    FractalUtils is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FractalUtils is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FractalUtils.  If not, see <https://www.gnu.org/licenses/>.

    Contact with me:
    github:https://github.com/ToKiNoBug
*/

#include <stdio.h>

#include "fractal_colors.h"

int main() {
  const uint32_t val =
      fractal_utils::color_u8c4(0.9, fractal_utils::color_series::pink);

  printf("val = 0x%X\n", val);

  const char *const name_of_autum = fractal_utils::color_series_enum_to_str(
      fractal_utils::color_series::parula);

  printf("value of parula = %i\n", int(fractal_utils::color_series::parula));

  printf("name of parula = %s\n", name_of_autum);

  printf("value of name of parula = %i\n",
         (int)fractal_utils::color_series_str_to_enum(
             fractal_utils::color_series_enum_to_str(
                 fractal_utils::color_series::parula)));

  return 0;
}