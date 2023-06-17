#include <multiprecision_utils.h>
#include <gmpxx.h>
#include <iostream>
#include <random>
#include "gmp_support.h"

using namespace fractal_utils;
using std::cout, std::endl;

constexpr double epsilon = 1e-28;

void test_add(gmp_complex_wrapper &a, const gmp_complex_wrapper &b,
              gmp_complex_wrapper &c, gmp_complex_buffer &buf) noexcept;

void test_subtract(gmp_complex_wrapper &a, const gmp_complex_wrapper &b,
                   gmp_complex_wrapper &c, gmp_complex_buffer &buf) noexcept;

void test_mult(gmp_complex_wrapper &a, const gmp_complex_wrapper &b,
               gmp_complex_wrapper &c, gmp_complex_buffer &buf) noexcept;

void test_divide(gmp_complex_wrapper &a, const gmp_complex_wrapper &b,
                 gmp_complex_wrapper &c, gmp_complex_buffer &buf) noexcept;

void test_inverse(gmp_complex_wrapper &a, gmp_complex_wrapper &c,
                  gmp_complex_buffer &buf) noexcept;

void test_square(gmp_complex_wrapper &a, gmp_complex_wrapper &c,
                 gmp_complex_buffer &buf) noexcept;

void test_power(gmp_complex_wrapper &a, int64_t n, gmp_complex_wrapper &c,
                gmp_complex_buffer &buf) noexcept;

void set_rand(gmp_complex_wrapper &dst,
              std::normal_distribution<double> &rand) noexcept {
  thread_local std::random_device rd;
  thread_local std::mt19937_64 mt{rd()};

  dst.real() = rand(mt);
  dst.imag() = rand(mt);
}

void *custom_alloc(size_t sz) {
  cout << "Allocated " << sz << " bytes\n";
  return malloc(sz);
}

void *(*impl_reallocate_function)(void *ptr, size_t old_size,
                                  size_t new_size) = nullptr;

void *custom_realloc_fun(void *ptr, size_t old_size, size_t new_size) {
  cout << "Reallocated from " << old_size << " bytes to " << new_size
       << " bytes\n";
  return ::impl_reallocate_function(ptr, old_size, new_size);
}

int main(int, char **) {
  mp_get_memory_functions(nullptr, &impl_reallocate_function, nullptr);
  mp_set_memory_functions(custom_alloc, custom_realloc_fun, nullptr);

  // complex_type_of<float_by_precision_t<8>> c8;
  //  complex_type_of<gmp_float_t> g{0}, f{2};
  // mpf_class m{3, 100};
  std::normal_distribution<double> rand{0, 1e8};
  const int prec = 500;
  gmp_complex_wrapper a{prec}, b{prec}, c{prec};
  auto reset = [&a, &b, &c, &rand]() {
    set_rand(a, rand);
    set_rand(b, rand);
    set_rand(c, rand);
  };

  gmp_complex_buffer buf{prec};

  reset();
  test_add(a, b, c, buf);
  reset();
  test_subtract(a, b, c, buf);
  reset();
  test_mult(a, b, c, buf);
  reset();
  test_divide(a, b, c, buf);

  reset();
  test_inverse(a, c, buf);
  reset();
  test_square(a, c, buf);

  reset();
  test_power(a, 4, c, buf);
  reset();
  test_power(a, -4, c, buf);
  reset();
  test_power(a, 3, c, buf);
  reset();
  test_power(a, -3, c, buf);
  reset();
  test_power(a, 2, c, buf);
  reset();
  test_power(a, -2, c, buf);
  reset();
  test_power(a, 1, c, buf);
  reset();
  test_power(a, -1, c, buf);
  reset();
  test_power(a, 0, c, buf);
}

void handle_result(std::complex<double> correct,
                   std::complex<double> result) noexcept {
  const double rerr = norm(correct - result) / norm(correct);
  if (rerr > epsilon) {
    cout << " -- incorrect, expected " << correct << ", rerror = " << rerr
         << endl;
    exit(1);
  } else {
    cout << endl;
  }
}

void test_add(gmp_complex_wrapper &a, const gmp_complex_wrapper &b,
              gmp_complex_wrapper &c, gmp_complex_buffer &buf) noexcept {
  auto correct = a.to_std_complex<double>() + b.to_std_complex<double>();
  a.add(b, c);
  cout << "(" << a << ") + (" << b << ") = " << c;

  auto result = c.to_std_complex<double>();

  handle_result(correct, result);

  correct = a.to_std_complex<double>() + c.to_std_complex<double>();
  cout << "(" << a << ") + (" << c << ") = " << a.add(c);
  result = a.to_std_complex<double>();

  handle_result(correct, result);
}

void test_subtract(gmp_complex_wrapper &a, const gmp_complex_wrapper &b,
                   gmp_complex_wrapper &c, gmp_complex_buffer &buf) noexcept {
  auto correct = a.to_std_complex<double>() - b.to_std_complex<double>();
  a.subtract(b, c);
  cout << "(" << a << ") - (" << b << ") = " << c;

  auto result = c.to_std_complex<double>();

  handle_result(correct, result);

  correct = a.to_std_complex<double>() - c.to_std_complex<double>();
  cout << "(" << a << ") - (" << c << ") = " << a.subtract(c);
  result = a.to_std_complex<double>();
  handle_result(correct, result);
}

void test_mult(gmp_complex_wrapper &a, const gmp_complex_wrapper &b,
               gmp_complex_wrapper &c, gmp_complex_buffer &buf) noexcept {
  auto correct = a.to_std_complex<double>() * b.to_std_complex<double>();
  a.mult(b, c, buf);
  cout << "(" << a << ") * (" << b << ") = " << c;

  auto result = c.to_std_complex<double>();

  handle_result(correct, result);

  correct = a.to_std_complex<double>() * c.to_std_complex<double>();
  cout << "(" << a << ") * (" << c << ") = " << a.mult(c, buf);
  result = a.to_std_complex<double>();

  handle_result(correct, result);
}

void test_divide(gmp_complex_wrapper &a, const gmp_complex_wrapper &b,
                 gmp_complex_wrapper &c, gmp_complex_buffer &buf) noexcept {
  auto correct = a.to_std_complex<double>() / b.to_std_complex<double>();
  a.divide(b, c, buf);
  cout << "(" << a << ") / (" << b << ") = " << c;

  auto result = c.to_std_complex<double>();

  handle_result(correct, result);

  correct = a.to_std_complex<double>() / c.to_std_complex<double>();
  cout << "(" << a << ") / (" << c << ") = " << a.divide(c, buf);
  result = a.to_std_complex<double>();

  handle_result(correct, result);
}

void test_inverse(gmp_complex_wrapper &a, gmp_complex_wrapper &c,
                  gmp_complex_buffer &buf) noexcept {
  auto correct = 1.0 / a.to_std_complex<double>();
  a.inverse(c, buf);
  cout << "1 / (" << a << ") = " << c;
  auto result = c.to_std_complex<double>();

  handle_result(correct, result);

  correct = 1.0 / a.to_std_complex<double>();
  cout << "1 / (" << a << ") = " << a.inverse(buf);

  handle_result(correct, result);
}

void test_square(gmp_complex_wrapper &a, gmp_complex_wrapper &c,
                 gmp_complex_buffer &buf) noexcept {
  auto correct = a.to_std_complex<double>() * a.to_std_complex<double>();
  a.square(c, buf);
  cout << "(" << a << ")^2 = " << c;
  auto result = c.to_std_complex<double>();

  handle_result(correct, result);

  correct = a.to_std_complex<double>() * a.to_std_complex<double>();
  cout << "(" << a << ")^2 = " << a.square(buf);
  result = a.to_std_complex<double>();

  handle_result(correct, result);
}

void test_power(gmp_complex_wrapper &a, int64_t n, gmp_complex_wrapper &c,
                gmp_complex_buffer &buf) noexcept {
  // a.real().
  auto correct = std::pow(a.to_std_complex<double>(), n);
  a.power(n, c, buf);
  cout << "(" << a << ")^" << n << " = " << c;
  auto result = c.to_std_complex<double>();

  handle_result(correct, result);

  correct = std::pow(a.to_std_complex<double>(), n);
  cout << "(" << a << ")^" << n << " = " << a.power(n, buf);
  result = a.to_std_complex<double>();

  handle_result(correct, result);
}