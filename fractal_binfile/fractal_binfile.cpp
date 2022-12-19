#include "fractal_binfile.h"
#include <stdint.h>

using namespace fractal_utils;
constexpr char __cmp[] = {'A', 'r', 'm', 'a', 'g', 'e', 'd', 'd',
                          'o', 'n', 6,   6,   6,   42,  30,  0};
fractal_utils::file_header::file_header() {
  memset(this->data, 0, sizeof(file_header));

  memcpy(this->data, __cmp, sizeof(__cmp));
}

bool fractal_utils::file_header::is_valid() const noexcept {

  for (int idx = 0; idx < sizeof(__cmp) / sizeof(__cmp[0]); idx++) {
    if (this->data[idx] != __cmp[idx]) {
      return false;
    }
  }
  return true;
}

bool fractal_utils::serialize_to_memory(
    const data_block *const src, const uint64_t block_num,
    const bool write_header, void *const dest_beg, const uint64_t dest_capacity,
    uint64_t *const dest_bytes_used,
    uint64_t *const __blocks_written) noexcept {

  if (dest_beg == nullptr || dest_capacity <= 0) {

    if (dest_bytes_used != nullptr) {
      *dest_bytes_used = 0;
    }

    if (__blocks_written != nullptr) {
      *__blocks_written = 0;
    }

    return false;
  }

  uint8_t *dest = reinterpret_cast<uint8_t *>(dest_beg);
  uint8_t *const dest_end = dest + dest_capacity;
  if (write_header) {

    if (dest_end - dest < sizeof(file_header)) {

      if (dest_bytes_used != nullptr) {
        *dest_bytes_used = 0;
      }

      if (__blocks_written != nullptr) {
        *__blocks_written = 0;
      }

      return false;
    }

    file_header h;

    memcpy(dest, &h, sizeof(h));
    dest += sizeof(h);
  }

  uint64_t blocks_finished = 0;

  for (blocks_finished = 0; blocks_finished < block_num; blocks_finished++) {

    uint64_t bytes_this_block = 0;
    bytes_this_block += sizeof(int64_t) + sizeof(uint64_t);

    bytes_this_block += src[blocks_finished].bytes;

    bool error = dest + bytes_this_block > dest_end;
    error = error || (src[blocks_finished].bytes > 0 &&
                      src[blocks_finished].data == nullptr);

    if (error) {
      // data capacity not enough

      if (dest_bytes_used != nullptr) {
        *dest_bytes_used = dest - reinterpret_cast<uint8_t *>(dest_beg);
      }

      if (__blocks_written != nullptr) {
        *__blocks_written = blocks_finished;
      }

      return false;
    }

    memcpy(dest, &src[blocks_finished].tag, sizeof(int64_t));
    dest += sizeof(int64_t);

    memcpy(dest, &src[blocks_finished].bytes, sizeof(uint64_t));
    dest += sizeof(uint64_t);
    if (src[blocks_finished].bytes) {

      memcpy(dest, src[blocks_finished].data, src[blocks_finished].bytes);
      dest += src[blocks_finished].bytes;
    }
  }

  if (dest_bytes_used != nullptr) {
    *dest_bytes_used = dest - reinterpret_cast<uint8_t *>(dest_beg);
  }

  if (__blocks_written != nullptr) {
    *__blocks_written = blocks_finished;
  }

  return true;
}

bool fractal_utils::parse_from_memory(
    const void *const __data, const uint64_t bytes,
    std::vector<data_block> *const data_blocks) noexcept {
  if (data_blocks == nullptr) {
    return false;
  }

  data_blocks->clear();

  const uint8_t *const data = reinterpret_cast<const uint8_t *>(__data);

  if (data == nullptr || bytes < sizeof(file_header)) {
    return false;
  }

  {
    const file_header *fhp = reinterpret_cast<const file_header *>(data);

    if (!fhp->is_valid())
      return false;
  }

  uint64_t offset = sizeof(file_header);

  bool error = false;

  while (true) {

    if (offset == bytes) {
      break;
    }

    if (offset > bytes) {
      error = true;
      break;
    }

    data_block blk;
    blk.tag = *(int64_t *)(data + offset);
    offset += sizeof(int64_t);

    blk.bytes = *(uint64_t *)(data + offset);
    offset += sizeof(uint64_t);

    if (offset > bytes) {
      error = true;
      break;
    }

    blk.file_offset = offset;
    blk.data = const_cast<uint8_t *>(data + offset);

    offset += blk.bytes;

    if (offset > bytes) {
      error = true;
      break;
    }

    data_blocks->emplace_back(blk);
  }

  if (error) {
    return false;
  }

  return true;
}