#include "fractal_map.h"

#include <memory>

using namespace libfractal_map;

fractal_map
libfractal_map::fractal_map::create(size_t rows, size_t cols,
                                    size_t sizeof_element) noexcept {
  fractal_map map;

  if (rows <= 0 || cols <= 0 || sizeof_element <= 0) {
    // no memory to be allocated
    map.data = nullptr;
  } else {

    map.call_free_on_destructor = true;
    map.data = malloc(rows * cols * sizeof_element);
  }

  map.rows = rows;
  map.cols = cols;
  map.element_bytes = sizeof_element;

  return map;
}

libfractal_map::fractal_map::~fractal_map() {
  if (this->call_free_on_destructor && this->data != nullptr) {
    free(this->data);
  }
}