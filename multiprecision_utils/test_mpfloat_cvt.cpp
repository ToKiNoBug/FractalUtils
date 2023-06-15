#include "multiprecision_utils.h"
#include <iostream>

using fractal_utils::float_by_precision_t;
using fractal_utils::uint_by_precision_t;

using namespace fractal_utils;

using std::cout, std::endl;

#define handle_error(value) \
  if (!(value)) {           \
    abort();                \
  }

template <int precision>
void test() noexcept {
  cout << "Testing precision = " << precision << "...";

  static_assert(is_valid_precision(precision));

  static_assert(precision_of_uint_v<uint_by_precision_t<precision>> ==
                precision);

  // const double original = precision / 3.0;

  // cout << "original = " << original;

  float_by_precision_t<precision> floatX = precision;
  floatX /= 3;

  cout << "floatX = " << floatX;
  static_assert(precision_of_float_v<decltype(floatX)> == precision);

  uint8_t buffer[precision * 4];

  auto encoded_bytes = encode_float(floatX, buffer);
  handle_error(encoded_bytes == precision * 4);

  auto decode_value = decode_float<decltype(floatX)>(buffer);

  handle_error(decode_value.has_value());
  cout << ", decoded value = " << decode_value.value();
  handle_error(decode_value.value() == floatX);

  cout << " -- [passed]" << endl;
}

int main(int, char**) {
  static_assert(std::is_same_v<uint_by_precision_t<1>, uint32_t>);
  static_assert(std::is_same_v<uint_by_precision_t<2>, uint64_t>);

  static_assert(std::is_same_v<float_by_precision_t<1>, float>);
  static_assert(std::is_same_v<float_by_precision_t<2>, double>);

#ifdef __GNUC__
  static_assert(std::is_same_v<uint_by_precision_t<4>, __uint128_t>);
  static_assert(std::is_same_v<float_by_precision_t<4>, __float128>);
#endif

  test<1>();
  test<2>();
  test<4>();
  test<8>();
  test<16>();
  test<32>();
  test<64>();
  test<128>();
  test<256>();
  test<512>();
  test<1024>();
  test<2048>();

  return 0;
}