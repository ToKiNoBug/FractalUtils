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

#ifndef FRACTALUTILS_FRACTAL_BINMAP_H
#define FRACTALUTILS_FRACTAL_BINMAP_H

#include <cstdlib>
#include <memory>
#include <stdint.h>
#include <vector>

namespace fractal_utils {

struct file_header {
  file_header();

  int8_t data[32];

  bool is_valid() const noexcept;
};

struct data_block {
  int64_t tag;
  uint64_t bytes;

  /// unused when serialization
  uint64_t file_offset;
  void *data{nullptr};
};

class binfile {
public:
  std::vector<data_block> blocks;

  void *(*callback_malloc)(size_t) = malloc;
  void (*callback_free)(void *) = free;

  void remove_all_blocks() noexcept;

  ~binfile();

  bool parse_from_file(const char *const filename,
                       const bool offset_only = false) noexcept;
};

bool parse_from_memory(const void *const data, const uint64_t bytes,
                       std::vector<data_block> *const data_blocks) noexcept;

/// returns true when is finished.
bool serialize_to_memory(const data_block *const blocks,
                         const uint64_t block_num, const bool write_header,
                         void *const dest, const uint64_t dest_capacity,
                         uint64_t *const dest_bytes_used,
                         uint64_t *const blocks_written) noexcept;

bool serialize_to_file(const data_block *const src, const uint64_t block_num,
                       const bool write_header,
                       const char *const filename) noexcept;

} // namespace fractal_utils

#endif // FRACTALUTILS_FRACTAL_BINMAP_H