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
#include <ctime>
#include <hex_convert.h>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using std::cout, std::endl;

int main() {
  srand(time(nullptr));
  std::random_device rd;
  std::mt19937 mt{rd()};

  std::uniform_real_distribution<double> rand(-1, 1);

  const size_t num = 1024;

  std::vector<double> randvec;
  std::vector<double> another;
  randvec.resize(num);

  for (double &val : randvec) {
    val = rand(mt);
  }

  std::string hex;
  hex.resize(randvec.size() * sizeof(double) * 2 + 2);

  auto ret =
      fractal_utils::bin_2_hex(randvec.data(), randvec.size() * sizeof(double),
                               hex.data(), hex.size(), true);

  if (!ret.has_value()) {
    cout << "Failed at line " << __LINE__ << endl;
    return 1;
  }

  hex.resize(ret.value());

  // cout << "Hex :" << hex << endl;

  another.resize(randvec.size());

  ret = fractal_utils::hex_2_bin(hex, another.data(),
                                 another.size() * sizeof(double));
  if (!ret.has_value()) {
    cout << "Failed at line " << __LINE__ << endl;
    return 1;
  }

  if (ret.value() != randvec.size() * sizeof(double)) {
    cout << "Size mismatch : " << ret.value() << " but expected "
         << randvec.size() * sizeof(double) << " bytes." << endl;
    return 1;
  }

  bool err = false;
  for (size_t i = 0; i < another.size(); i++) {
    if (another[i] != randvec[i]) {
      cout << "value changed at index " << i << std::endl;
      err = true;
    }
  }
  if (err) {
    cout << "Failed at line " << __LINE__ << std::endl;
  }

  cout << "Success" << endl;

  return 0;
}