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

#include "binary_archive.h"
#include <assert.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <string>

fractal_utils::data_segment::data_segment(int64_t tag, const variant_t &var)
    : m_tag(tag), m_variant(var) {}
fractal_utils::data_segment::data_segment(int64_t tag, variant_t &&var)
    : m_tag(tag), m_variant(var) {}

bool fractal_utils::data_segment::has_ownership() const noexcept {
  switch (this->m_variant.index()) {
  case 0:
    return true;
  case 1:
    return false;
  case 2:
    return false;
  }
  assert(false);
  return false;
}

uint64_t fractal_utils::data_segment::bytes() const noexcept {

  switch (this->m_variant.index()) {
  case 0:
    return std::get<0>(this->m_variant).size();
  case 1:
    return std::get<1>(this->m_variant).size();
  case 2:
    return std::get<2>(this->m_variant).bytes;
  }
  assert(false);
  return 0;
}

void *fractal_utils::data_segment::impl_data() const noexcept {
  switch (this->m_variant.index()) {
  case 0:
    return (void *)std::get<0>(this->m_variant).data();
  case 1:
    return (void *)std::get<1>(this->m_variant).data();
  case 2:
    return nullptr;
  default:
    assert(false);
    return nullptr;
  }
}

inline bool read_size_correct(std::istream &is, char *data,
                              size_t len) noexcept {
  is.read(data, len);
  const size_t read_bytes = is.gcount();
  return read_bytes == len;
}

template <typename T> bool read_val_check(std::istream &is, T &t) noexcept {
  return read_size_correct(is, reinterpret_cast<char *>(&t), sizeof(T));
}

std::optional<fractal_utils::data_segment>
fractal_utils::read_segment_data(std::istream &is) noexcept {
  return read_segment_data(is, {}, nullptr);
}

std::optional<fractal_utils::data_segment>
fractal_utils::read_segment_data(std::istream &is, std::span<uint8_t> buffer,
                                 size_t *used_bytes_dest) noexcept {
  fractal_utils::data_segment ret;
  int64_t tag;
  if (!read_val_check(is, tag)) {
    return std::nullopt;
  }

  uint64_t bytes{0};
  if (!read_val_check(is, bytes)) {
    return std::nullopt;
  }
  ret.set_tag(tag);

  bool use_buffer{false};

  const uint64_t bytes_required = bytes;
  if (bytes_required > buffer.size_bytes()) {
    use_buffer = false;
  } else {
    use_buffer = true;
  }

  // this vector is used when buffer is not enough
  std::vector<uint8_t> vec_may_use;
  uint8_t *data_ptr{nullptr};

  if (use_buffer) {
    data_ptr = buffer.data();
  } else {
    vec_may_use.resize(bytes);
    data_ptr = vec_may_use.data();
  }

  if (!read_size_correct(is, reinterpret_cast<char *>(data_ptr), bytes)) {
    return std::nullopt;
  }

  if (use_buffer) {
    ret.set_variant(std::span<uint8_t>(data_ptr, bytes));
  } else {
    ret.set_variant(std::move(vec_may_use));
  }

  if (used_bytes_dest != nullptr) {
    if (use_buffer) {
      *used_bytes_dest = bytes;
    } else {
      *used_bytes_dest = 0;
    }
  }

  return ret;
}

std::string fractal_utils::binary_archive::load(std::istream &is) noexcept {
  return this->load(is, {}, nullptr);
}

std::string
fractal_utils::binary_archive::load(std::istream &is, std::span<uint8_t> buffer,
                                    size_t *used_bytes_dest) noexcept {

  this->m_segments.clear();

  if (!read_val_check(is, this->m_header)) {
    return "Failed to read the header";
  }

  const bool has_buffer = (buffer.size() > 0);

  uint8_t *const buffer_beg = buffer.data();
  uint8_t *const buffer_end = buffer.data() + buffer.size();

  uint8_t *buffer_cur = buffer_beg;

  // size_t offset = sizeof(this->m_header);

  while (true) {

    if (is.eof() || is.peek() == EOF) {
      break;
    }

    if (is.fail()) {
      return "The input stream failed (is.fail() == true)";
    }
    if (is.bad()) {
      return "The input stream is bad (is.bad() == true)";
    }

    std::optional<fractal_utils::data_segment> seg_opt{std::nullopt};

    if (has_buffer) {
      size_t used_this_time{0};
      seg_opt =
          read_segment_data(is, {buffer_cur, buffer_end}, &used_this_time);
      buffer_cur += used_this_time;

    } else {
      seg_opt = read_segment_data(is);
    }

    if (!seg_opt.has_value()) {
      return fmt::format("Block {} can not be read. Input may be incomplete.",
                         this->m_segments.size());
    }

    // offset += sizeof(int64_t) + sizeof(uint64_t);
    //  seg_opt.value().set_offset(offset);
    // offset += seg_opt.value().bytes();
    this->m_segments.emplace_back(std::move(seg_opt.value()));
  }

  if (used_bytes_dest != nullptr) {
    if (has_buffer) {
      *used_bytes_dest = buffer_cur - buffer_beg;
    } else {
      *used_bytes_dest = 0;
    }
  }

  return {};
}

std::string
fractal_utils::binary_archive::load(std::string_view filename) noexcept {
  std::ifstream ifs{filename.data(), std::ios::binary};

  if (!ifs) {
    return fmt::format("Failed to open file {}", filename.data());
  }

  auto ret = this->load(ifs);
  ifs.close();
  return ret;
}
std::string
fractal_utils::binary_archive::load(std::string_view filename,
                                    std::span<uint8_t> buffer,
                                    size_t *used_bytes_dest) noexcept {

  std::ifstream ifs{filename.data(), std::ios::binary};

  if (!ifs) {
    return fmt::format("Failed to open file {}", filename.data());
  }

  auto ret = this->load(ifs, buffer, used_bytes_dest);
  ifs.close();
  return ret;
}

bool write_size_correct(std::ostream &os, const char *data,
                        size_t bytes) noexcept {
  try {
    os.write(data, bytes);
  } catch (...) {
    return false;
  }
  return true;
}

template <typename T>
bool write_val_correct(std::ostream &os, const T &t) noexcept {
  return write_size_correct(os, reinterpret_cast<const char *>(&t), sizeof(T));
}

bool fractal_utils::write_segment_data(std::ostream &os,
                                       const data_segment &ds) noexcept {
  if (!ds.has_data()) {
    return false;
  }

  if (!write_val_correct(os, ds.tag())) {
    return false;
  }

  if (!write_val_correct(os, ds.bytes())) {
    return false;
  }

  if (!write_size_correct(os, reinterpret_cast<const char *>(ds.data()),
                          ds.bytes())) {
    return false;
  }
  return true;
}

std::string
fractal_utils::binary_archive::save(std::ostream &os) const noexcept {
  if (!write_val_correct(os, this->m_header)) {
    return "Failed to write header";
  }

  for (size_t idx = 0; idx < this->m_segments.size(); idx++) {
    if (!write_segment_data(os, this->m_segments[idx])) {
      return fmt::format("Failed to write segment {}", idx);
    }
  }
  return {};
}

std::string
fractal_utils::binary_archive::save(std::string_view filename) const noexcept {
  std::ofstream ofs{filename.data(), std::ios::binary};

  if (!ofs) {
    return fmt::format("Failed to open or create {}.", filename);
  }
  auto ret = this->save(ofs);
  ofs.close();
  return ret;
}

std::optional<size_t>
fractal_utils::binary_archive::impl_find_first_of(int64_t tag) const noexcept {
  for (size_t i = 0; i < this->m_segments.size(); i++) {
    if (this->m_segments[i].tag() == tag) {
      return i;
    }
  }
  return std::nullopt;
}

std::optional<size_t>
fractal_utils::binary_archive::impl_find_last_of(int64_t tag) const noexcept {
  for (ptrdiff_t i = this->m_segments.size() - 1; i >= 0; i--) {
    if (this->m_segments[i].tag() == tag) {
      return i;
    }
  }
  return std::nullopt;
}

fractal_utils::data_segment *
fractal_utils::binary_archive::find_first_of(int64_t tag) noexcept {
  auto opt = this->impl_find_first_of(tag);
  if (opt.has_value()) {
    return &this->m_segments[opt.value()];
  }
  return nullptr;
}
const fractal_utils::data_segment *
fractal_utils::binary_archive::find_first_of(int64_t tag) const noexcept {
  auto opt = this->impl_find_first_of(tag);
  if (opt.has_value()) {
    return &this->m_segments[opt.value()];
  }
  return nullptr;
}

fractal_utils::data_segment *
fractal_utils::binary_archive::find_last_of(int64_t tag) noexcept {
  auto opt = this->impl_find_last_of(tag);
  if (opt.has_value()) {
    return &this->m_segments[opt.value()];
  }
  return nullptr;
}

const fractal_utils::data_segment *
fractal_utils::binary_archive::find_last_of(int64_t tag) const noexcept {
  auto opt = this->impl_find_last_of(tag);
  if (opt.has_value()) {
    return &this->m_segments[opt.value()];
  }
  return nullptr;
}