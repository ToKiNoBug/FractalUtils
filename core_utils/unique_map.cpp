#include "unique_map.h"

#include <cstring>

using namespace fractal_utils;

unique_map::unique_map(const unique_map &src) : internal::map_base{src} {
  this->m_data.reset(allocate_memory_aligned(64, src.bytes()));
  if (src.bytes() > 0)
    memcpy(this->m_data.get(), src.m_data.get(), src.bytes());
  this->m_capacity = src.bytes();
}

unique_map::unique_map(unique_map &&src)
    : internal::map_base{src},
      m_data{std::move(src.m_data)},
      m_capacity{src.capacity_bytes()} {
  src.reset(0, 0, src.element_bytes());
  src.m_capacity = 0;
}

unique_map::unique_map(internal::map_base base) : internal::map_base{base} {
  this->m_data.reset(allocate_memory_aligned(64, base.bytes()));
  this->m_capacity = base.bytes();
}

unique_map::unique_map(size_t r, size_t c, size_t ele_bytes)
    : internal::map_base{r, c, ele_bytes} {
  this->m_data.reset(allocate_memory_aligned(64, r * c * ele_bytes));
  this->m_capacity = r * c * ele_bytes;
}

unique_map &unique_map::operator=(const unique_map &src) & noexcept {
  this->reset(src);
  if (src.bytes() > 0) {
    memcpy(this->m_data.get(), src.data(), src.bytes());
  }
  return *this;
}

unique_map &unique_map::operator=(unique_map &&src) & noexcept {
  this->m_data = std::move(src.m_data);
  this->m_capacity = src.m_capacity;
  src.m_capacity = 0;
  static_cast<internal::map_base &>(*this) = src;
  src.reset(0, 0, this->element_bytes());
  return *this;
}

unique_map::operator fractal_map() noexcept {
  return fractal_map{map_view{*this}};
}

unique_map::operator const fractal_map() const noexcept {
  return fractal_map{constant_view{*this}};
}

void unique_map::resize(size_t r, size_t c) noexcept {
  this->reset(r, c, this->element_bytes());
}

void unique_map::reset(size_t r, size_t c, size_t ele_bytes) noexcept {
  const size_t old_bytes = this->bytes();

  const internal::map_base new_base{r, c, ele_bytes};
  if (new_base.bytes() <= 0 || new_base.bytes() <= this->capacity()) {
    static_cast<internal::map_base &>(*this) = new_base;
    return;
  }

  // if the matrix is not empty
  this->reserve_bytes(new_base.bytes());
  static_cast<internal::map_base &>(*this) = new_base;
  const size_t new_bytes = this->bytes();
  if (old_bytes < new_bytes) {
    memset(reinterpret_cast<uint8_t *>(this->data()) + old_bytes, 0,
           new_bytes - old_bytes);
  }

  static_cast<internal::map_base &>(*this) = new_base;
}

void unique_map::reserve(size_t sz) noexcept {
  this->reserve_bytes(sz * this->m_ele_bytes);
}

void unique_map::reserve_bytes(size_t bytes) noexcept {
  assert(this->capacity_bytes() >= this->bytes());
  if (bytes <= this->capacity_bytes()) {
    return;
  }

  void *new_data = allocate_memory_aligned(64, bytes);

  memcpy(new_data, this->data(), this->bytes());

  this->m_data.reset(new_data);
  this->m_capacity = bytes;
}

void unique_map::shrink_to_fit() noexcept {
  assert(this->capacity_bytes() >= this->bytes());
  if (this->capacity_bytes() == this->bytes()) {
    return;
  }

  void *new_data = allocate_memory_aligned(64, this->bytes());
  memcpy(new_data, this->data(), this->bytes());
  this->m_data.reset(new_data);
  this->m_capacity = this->bytes();
}

///////////////////////////////////////////////////////////

map_view::map_view(void *_data, size_t r, size_t c, size_t eleb)
    : internal::map_base{r, c, eleb}, m_data{_data} {}

map_view::map_view(unique_map &src)
    : m_data(src.data()), internal::map_base{src} {}

map_view::map_view(fractal_map &src)
    : m_data{src.data},
      internal::map_base{src.rows, src.cols, src.element_bytes} {}

map_view::operator fractal_map() noexcept {
  return fractal_map{this->rows(), this->cols(),
                     (uint32_t)this->element_bytes(), this->data()};
}

map_view::operator const fractal_map() const noexcept {
  return fractal_map{constant_view{*this}};
}

///////////////////////////////////////////////////////////

constant_view::constant_view(const void *_data, size_t r, size_t c, size_t eleb)
    : m_data{_data}, internal::map_base{r, c, eleb} {}

constant_view::constant_view(const unique_map &src)
    : m_data{src.data()}, internal::map_base{src} {}

constant_view::constant_view(const map_view &src)
    : m_data{src.data()}, internal::map_base{src} {}

constant_view::constant_view(const fractal_map &src)
    : m_data{src.data},
      internal::map_base{src.rows, src.cols, src.element_bytes} {}

constant_view::operator const fractal_map() const noexcept {
  return fractal_map{this->rows(), this->cols(),
                     (uint32_t)this->element_bytes(),
                     const_cast<void *>(this->data())};
}

/*
void fractal_utils::copy(constant_view src, unique_map &dst) noexcept {
  static_cast<internal::map_base &>(dst) = src;

  dst.m_data.reset(allocate_memory_aligned(64, src.bytes()));
  memcpy(dst.data(), dst.m_data.get(), src.bytes());
}
*/