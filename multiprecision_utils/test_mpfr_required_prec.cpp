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

#include <iostream>
#include <mpfr_support.h>

using std::cin, std::cout, std::endl;

int main() noexcept {
  fractal_utils::center_wind<boostmp::mpfr_float> wind;

  cout << "center[0] = ";
  cin >> wind.center[0];
  cout << "center[1] = ";
  cin >> wind.center[1];

  cout << "x_span = ";
  cin >> wind.x_span;
  cout << "y_span = ";
  cin >> wind.y_span;

  int rows, cols;
  cout << "rows = ";
  cin >> rows;
  cout << "cols = ";
  cin >> cols;

  cout << "Minimum required precision is "
       << fractal_utils::required_precision_of(wind, rows, cols) << endl;

  return 0;
}