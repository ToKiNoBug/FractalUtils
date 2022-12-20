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