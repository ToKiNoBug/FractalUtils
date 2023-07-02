/*
Copyright © 2022-2023  TokiNoBug
This file is part of FractalUtils.

FractalUtils is free software: you can redistribute it and/or modify it under
                                                                   the terms of
the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

                                       FractalUtils is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

`  You should have received a copy of the GNU General Public License
 along with FractalUtils.  If not, see <https://www.gnu.org/licenses/>.

 Contact with me:
 github:https://github.com/ToKiNoBug
*/
#include <fractal_map.h>
#include <iostream>

using std::cout, std::endl;

int main() {
  const int png_count = 60;
  const double ratio = 2;
  const int rows = 1080;

  const int png_idx_max = 120;

  for (int pid = 0; pid < png_idx_max; pid++) {
    cout << "pid = " << pid << ", skip rows = "
         << fractal_utils::skip_rows(rows, ratio, png_count, pid) << '\n';
  }
  cout << endl;
  return 0;
}