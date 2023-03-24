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