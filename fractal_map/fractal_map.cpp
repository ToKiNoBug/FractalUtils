#include "fractal_map.h"

#include <cstdlib>
#include <memory>

using namespace fractal_utils;

fractal_map fractal_utils::fractal_map::create(size_t rows, size_t cols,
                                               size_t sizeof_element) noexcept {
  fractal_map map;

  if (rows <= 0 || cols <= 0 || sizeof_element <= 0) {
    // no memory to be allocated
    map.data = nullptr;
  } else {

    map.call_free_on_destructor = true;

    map.data =
#ifdef _WIN32
        _aligned_malloc(rows * cols * sizeof_element, 128);
#else
        aligned_alloc(128, rows * cols * sizeof_element);
#endif
  }

  map.rows = rows;
  map.cols = cols;
  map.element_bytes = sizeof_element;

  return map;
}

fractal_utils::fractal_map::~fractal_map() {
  if (this->call_free_on_destructor && this->data != nullptr) {
    free(this->data);
  }
}