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