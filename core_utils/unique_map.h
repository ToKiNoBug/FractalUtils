#ifndef FRACTALUTILS_COREUTILS_UNIQUEMAP_H
#define FRACTALUTILS_COREUTILS_UNIQUEMAP_H

#include <cassert>
#include <cstdlib>
#include <memory>
#include <ranges>

#include "fractal_map.h"

namespace fractal_utils {

namespace internal {
class map_base {
 protected:
  size_t m_rows{0};
  size_t m_cols{0};
  size_t m_ele_bytes{0};

 public:
  map_base() = default;
  map_base(size_t r, size_t c, size_t ele_bytes)
      : m_rows(r), m_cols(c), m_ele_bytes(ele_bytes) {}
  inline size_t rows() const noexcept { return this->m_rows; }
  inline size_t cols() const noexcept { return this->m_cols; }
  inline size_t element_bytes() const noexcept { return this->m_ele_bytes; }
  inline size_t size() const noexcept { return this->m_rows * this->m_cols; }
  inline size_t bytes() const noexcept {
    return this->m_rows * this->m_cols * this->m_ele_bytes;
  }

  [[deprecated]] inline auto byte_count() const noexcept {
    return this->bytes();
  }
  [[deprecated]] inline auto element_count() const noexcept {
    return this->size();
  }

  inline bool operator==(const map_base &B) const noexcept {
    return (this->m_rows == B.m_rows) && (this->m_cols == B.m_cols) &&
           (this->m_ele_bytes == B.m_ele_bytes);
  }

  inline bool operator!=(const map_base &B) const noexcept {
    return !this->operator==(B);
  }
};

#define FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CONST_ACCESSER_MEMBER_FUNCTIONS      \
  const void *data() const noexcept { return this->impl_data(); }             \
                                                                              \
  template <typename T>                                                       \
  const T *address(size_t idx) const noexcept {                               \
    static_assert(!std::is_same_v<T, void>, "T should not be void");          \
    assert(sizeof(T) == static_cast<const derived *>(this)->element_bytes()); \
    return reinterpret_cast<const T *>(this->impl_data()) + idx;              \
  }                                                                           \
                                                                              \
  template <typename T>                                                       \
  const T *address(size_t r, size_t c) const noexcept {                       \
    static_assert(!std::is_same_v<T, void>, "T should not be void");          \
    assert(sizeof(T) == static_cast<const derived *>(this)->element_bytes()); \
    this->assert_for_size(r, c);                                              \
    return this->address<T>(                                                  \
        r * reinterpret_cast<const derived *>(this)->cols() + c);             \
  }                                                                           \
                                                                              \
  template <typename T>                                                       \
  const T &at(size_t r, size_t c) const noexcept {                            \
    static_assert(!std::is_same_v<T, void>, "T should not be void");          \
    this->assert_for_size(r, c);                                              \
    this->assert_for_ele_bytes(sizeof(T));                                    \
    return *this->address<T>(r, c);                                           \
  }                                                                           \
                                                                              \
  template <typename T>                                                       \
  const T &at(size_t idx) const noexcept {                                    \
    static_assert(!std::is_same_v<T, void>, "T should not be void");          \
    this->assert_for_size(idx);                                               \
    this->assert_for_ele_bytes(sizeof(T));                                    \
    return *this->address<T>()[idx];                                          \
  }                                                                           \
                                                                              \
  template <typename T>                                                       \
  inline std::ranges::subrange<const T *> items() const noexcept {            \
    this->assert_for_ele_bytes(sizeof(T));                                    \
    return std::ranges::subrange<const T *>(                                  \
        this->address<T>(0), this->address<T>(0) + this->size());             \
  }

// implement const data access
template <class derived>
class const_map_accesser {
 private:
  const void *impl_data() const noexcept {
    return static_cast<const derived *>(this)->impl_get_data_for_accesser();
  }

 private:
  void assert_for_size(size_t r, size_t c) const noexcept {
    assert(r < static_cast<const derived *>(this)->rows());
    assert(c < static_cast<const derived *>(this)->cols());
  }

  void assert_for_size(size_t idx) const noexcept {
    assert(idx < static_cast<const derived *>(this)->size());
  }

  void assert_for_ele_bytes(size_t element_bytes) const noexcept {
    assert(element_bytes ==
           static_cast<const derived *>(this)->element_bytes());
  }

 public:
  FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CONST_ACCESSER_MEMBER_FUNCTIONS
};

// implement data access
template <class derived>
class map_accesser {
 private:
  void *impl_data() const noexcept {
    return static_cast<const derived *>(this)->impl_get_data_for_accesser();
  }

  inline void assert_for_size(size_t r, size_t c) const noexcept {
    assert(r < static_cast<const derived *>(this)->rows());
    assert(c < static_cast<const derived *>(this)->cols());
  }

  inline void assert_for_size(size_t idx) const noexcept {
    assert(idx < static_cast<const derived *>(this)->size());
  }

  inline void assert_for_ele_bytes(size_t element_bytes) const noexcept {
    assert(element_bytes ==
           static_cast<const derived *>(this)->element_bytes());
  }

 public:
  void *data() noexcept { return this->impl_data(); }
  template <typename T>
  T *address(size_t idx) noexcept {
    static_assert(!std::is_same_v<T, void>, "T should not be void");
    assert(sizeof(T) == static_cast<derived *>(this)->element_bytes());
    this->assert_for_size(idx);
    return reinterpret_cast<T *>(this->impl_data()) + idx;
  }

  template <typename T>
  T *address(size_t r, size_t c) noexcept {
    static_assert(!std::is_same_v<T, void>, "T should not be void");
    assert(sizeof(T) == static_cast<derived *>(this)->element_bytes());
    this->assert_for_size(r, c);
    return this->address<T>(
        r * reinterpret_cast<const derived *>(this)->cols() + c);
  }

  template <typename T>
  T &at(size_t r, size_t c) noexcept {
    static_assert(!std::is_same_v<T, void>, "T should not be void");
    this->assert_for_size(r, c);
    this->assert_for_ele_bytes(sizeof(T));
    return *this->address<T>(r, c);
  }

  template <typename T>
  T &at(size_t idx) noexcept {
    static_assert(!std::is_same_v<T, void>, "T should not be void");
    this->assert_for_size(idx);
    this->assert_for_ele_bytes(sizeof(T));
    return *this->address<T>()[idx];
  }

  template <typename T>
  inline std::ranges::subrange<T *> items() noexcept {
    this->assert_for_ele_bytes(sizeof(T));
    return std::ranges::subrange<T *>(this->address<T>(0),
                                      this->address<T>(0) + this->size());
  }

  FRACTAL_UTILS_PRIVATE_MACRO_MAKE_CONST_ACCESSER_MEMBER_FUNCTIONS
};

class void_deleter {
 public:
  void operator()(void *data) const noexcept { free_memory_aligned(data); }
};

template <class T>
concept map_like = requires(T t) {
  t.rows();
  t.cols();
  t.element_bytes();
  t.size();
  t.data();
};

using const_fractal_map_t = const fractal_map;
}  // namespace internal

class unique_map : public internal::map_accesser<unique_map>,
                   // public internal::const_map_accesser<unique_map>,
                   public internal::map_base {
 private:
  std::unique_ptr<void, internal::void_deleter> m_data{nullptr};
  size_t m_capacity{0};

  template <class T>
  friend class internal::map_accesser;
  template <class T>
  friend class internal::const_map_accesser;

  void *impl_get_data_for_accesser() const noexcept {
    return this->m_data.get();
  }

 public:
  unique_map() = default;
  unique_map(unique_map &&);
  unique_map(const unique_map &);

  explicit unique_map(internal::map_base);
  unique_map(size_t r, size_t c, size_t ele_bytes);
  explicit operator fractal_map() noexcept;
  explicit operator internal::const_fractal_map_t() const noexcept;

  unique_map &operator=(const unique_map &src) & noexcept;
  unique_map &operator=(unique_map &&src) & noexcept;

  inline size_t capacity() const noexcept { return this->m_capacity; }
  inline size_t capacity_bytes() const noexcept {
    return this->m_capacity * this->element_bytes();
  }

  consteval bool own_memory() const noexcept { return true; }
  consteval bool has_ownership() const noexcept { return true; }

  void resize(size_t r, size_t c) noexcept;
  void reset(size_t r, size_t c, size_t ele_bytes) noexcept;
  inline void reset(internal::map_base mb) noexcept {
    this->reset(mb.rows(), mb.cols(), mb.element_bytes());
  }

  void reserve(size_t sz) noexcept;
  inline void reserve(size_t r, size_t c) noexcept { this->reserve(r * c); }

  void reserve_bytes(size_t bytes) noexcept;

  void shrink_to_fit() noexcept;

  auto &unwrap_ptr() noexcept { return this->m_data; }

  template <class T>
  unique_map &operator=(T &src) noexcept
    requires internal::map_like<T>
  {
    this->reset(src.rows(), src.cols(), src.element_bytes());
    memcpy(this->data(), src.data(), src, bytes());
    return *this;
  }
};

class map_view : public internal::map_accesser<map_view>,
                 public internal::map_base {
 private:
  void *const m_data;

  template <class T>
  friend class internal::map_accesser;
  template <class T>
  friend class internal::const_map_accesser;

  void *impl_get_data_for_accesser() const noexcept { return this->m_data; }

 public:
  map_view() = delete;
  map_view(const map_view &) = default;
  map_view(map_view &&) = default;
  map_view(void *_data, size_t r, size_t c, size_t eleb);

  template <typename T>
  map_view(T *_data, size_t r, size_t c) : map_view{_data, r, c, sizeof(T)} {}

  map_view(unique_map &src);

  map_view(fractal_map &src);

  explicit operator fractal_map() noexcept;
  explicit operator internal::const_fractal_map_t() const noexcept;

  consteval bool own_memory() const noexcept { return false; }
  consteval bool has_ownership() const noexcept { return false; }
};

class constant_view : public internal::const_map_accesser<constant_view>,
                      public internal::map_base {
 private:
  const void *const m_data;
  const void *impl_get_data_for_accesser() const noexcept {
    return this->m_data;
  }
  // template <class T> friend class internal::map_accesser;
  template <class T>
  friend class internal::const_map_accesser;

 public:
  constant_view() = delete;
  constant_view(const constant_view &) = default;
  constant_view(constant_view &&) = default;

  constant_view(const void *_data, size_t r, size_t c, size_t eleb);

  template <typename T>
  constant_view(const T *_data, size_t r, size_t c)
      : constant_view{_data, r, c, sizeof(T)} {}

  constant_view(const unique_map &src);
  constant_view(const map_view &src);
  constant_view(const fractal_map &src);

  explicit operator internal::const_fractal_map_t() const noexcept;

  consteval bool own_memory() const noexcept { return false; }
  consteval bool has_ownership() const noexcept { return false; }
};

}  // namespace fractal_utils

#endif  // FRACTALUTILS_COREUTILS_UNIQUEMAP_H