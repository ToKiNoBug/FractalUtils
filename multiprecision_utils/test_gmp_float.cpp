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

#include "gmp_support.h"
#include <iostream>
#include "hex_convert.h"
#include "multiprecision_utils.h"

using std::cout, std::endl;
using namespace fractal_utils;

void test_gmp(int gmp_precision) noexcept {
  gmp_float_t flt;
  flt.precision(gmp_precision);
  flt = -1;
  flt /= 3;

  cout << "flt = " << flt << endl;

  cout << "precision of flt = " << flt.precision() << endl;

  auto binary = fractal_utils::encode_gmp_float(flt);

  std::string buffer;
  buffer.resize(2 * fractal_utils::required_bytes_of(flt) + 1024);

  const auto str_bytes = bin_2_hex(binary, buffer, false);
  buffer.resize(str_bytes.value());

  cout << "binary of flt = " << buffer << endl;
  cout << "Length = " << buffer.size() << endl;

  auto decode_opt = decode_float<gmp_float_t>(binary);
  cout << "decode_opt.has_value() = " << decode_opt.has_value() << endl;

  if (decode_opt.has_value()) {
    cout << "decoded precision = " << decode_opt.value().precision() << endl;
    cout << "decoded value = " << decode_opt.value() << endl;
    if (decode_opt.value() != flt) {
      exit(1);
    }
  } else {
    exit(1);
  }

  auto encode_ret = encode_float(decode_opt.value(), binary);
  assert(encode_ret != 0);
  assert(binary.size() == encode_ret);
}

int main(int, char**) {
  fractal_utils::center_wind<fractal_utils::gmp_float_t> test;
  for (int p = 50; p < 2000; p++) {
    test_gmp(p);
  }

  return 0;
}