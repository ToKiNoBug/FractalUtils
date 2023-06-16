//
// Created by joseph on 6/16/23.
//

#include "gmp_support.h"
#include <iostream>
#include "hex_convert.h"

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

  auto decode_opt = decode_gmp_float(binary);
  cout << "decode_opt.has_value() = " << decode_opt.has_value() << endl;

  if (decode_opt.has_value()) {
    cout << "decoded precision = " << decode_opt.value().precision() << endl;
    cout << "decoded value = " << decode_opt.value() << endl;
    if (decode_opt.value() != flt) {
      abort();
    }
  } else {
    abort();
  }
}

int main(int, char**) {
  for (int p = 50; p < 2000; p++) {
    test_gmp(p);
  }

  return 0;
}