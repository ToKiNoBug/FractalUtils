/*
 Copyright © 2022-2023  TokiNoBug
This file is part of FractalUtils.

    FractalUtils is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FractalUtils is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FractalUtils.  If not, see <https://www.gnu.org/licenses/>.

    Contact with me:
    github:https://github.com/ToKiNoBug
*/
#ifndef FRACTALUTILS_COREUTILS_BINARCHIVE_H
#define FRACTALUTILS_COREUTILS_BINARCHIVE_H

#include <istream>
#include <optional>
#include <ostream>
#include <span>
#include <stddef.h>
#include <stdint.h>
#include <variant>
#include <vector>

namespace fractal_utils {

struct file_header {
  file_header();

  int8_t data[32];

  int8_t *custom_part() noexcept;
  const int8_t *custom_part() const noexcept;

  static size_t custom_part_len() noexcept;

  bool is_valid() const noexcept;
};

class data_segment {
public:
  struct segment_length {
    uint64_t bytes{0};
  };
  using variant_t =
      std::variant<std::vector<uint8_t>, std::span<uint8_t>, segment_length>;

private:
  int64_t m_tag{INT64_MAX};
  variant_t m_variant;
  // uint64_t m_offset{0};

public:
  data_segment() = default;
  data_segment(int64_t tag, const variant_t &var);
  data_segment(int64_t tag, variant_t &&var);

  inline int64_t tag() const noexcept { return this->m_tag; }
  inline void set_tag(int64_t _tag) noexcept { this->m_tag = _tag; }
  // inline uint64_t offset() const noexcept { return this->m_offset; }
  // inline void set_offset(uint64_t _offset) noexcept {    this->m_offset =
  // _offset;  }

  inline const auto &variant() const noexcept { return this->m_variant; }
  inline auto &variant() noexcept { return this->m_variant; }
  template <class T> inline void set_variant(T &&t) noexcept {
    this->m_variant = t;
  }

  bool has_ownership() const noexcept;

  inline bool has_data() const noexcept { return this->variant().index() != 2; }

  uint64_t bytes() const noexcept;

  inline void *data() noexcept { return this->impl_data(); }
  inline const void *data() const noexcept { return this->impl_data(); }

private:
  void *impl_data() const noexcept;
};

std::optional<data_segment> read_segment_data(std::istream &is) noexcept;
std::optional<data_segment> read_segment_data(std::istream &is,
                                              std::span<uint8_t> buffer,
                                              size_t *used_bytes_dest) noexcept;

bool write_segment_data(std::ostream &os, const data_segment &) noexcept;

class binary_archive {
private:
  file_header m_header;
  std::vector<data_segment> m_segments;

public:
  auto &header() noexcept { return this->m_header; }
  const auto &header() const noexcept { return this->m_header; }
  void set_header(const file_header &fh) noexcept { this->m_header = fh; }

  auto &segments() noexcept { return this->m_segments; }
  const auto &segments() const noexcept { return this->m_segments; }

  std::string load(std::istream &is) noexcept;
  std::string load(std::istream &is, std::span<uint8_t> buffer,
                   size_t *used_bytes_dest) noexcept;

  std::string load(std::string_view filename) noexcept;
  std::string load(std::string_view filename, std::span<uint8_t> buffer,
                   size_t *used_bytes_dest) noexcept;

  std::string save(std::ostream &os) const noexcept;
  std::string save(std::string_view filename) const noexcept;

  data_segment *find_first_of(int64_t tag) noexcept;
  const data_segment *find_first_of(int64_t tag) const noexcept;

  data_segment *find_last_of(int64_t tag) noexcept;
  const data_segment *find_last_of(int64_t tag) const noexcept;

private:
  std::optional<size_t> impl_find_first_of(int64_t tag) const noexcept;
  std::optional<size_t> impl_find_last_of(int64_t tag) const noexcept;
};
}; // namespace fractal_utils

#endif // FRACTALUTILS_COREUTILS_BINARCHIVE_H
