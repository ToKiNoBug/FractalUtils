#include "fractal_binfile.h"
#include <stdint.h>

#include <stdio.h>

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

bool fractal_utils::serialize_to_file(const data_block *const src,
                                      const uint64_t block_num,
                                      const bool write_header,
                                      const char *const filename) noexcept {
  if (filename == nullptr) {
    return false;
  }

  FILE *fp;
#ifdef _WIN32
  fopen_s(&fp, filename, "wb");
#else
  fp = fopen(filename, "wb");
#endif

  if (fp == nullptr) {
    return false;
  }

  if (write_header) {
    file_header fhd;
    fwrite(&fhd, 1, sizeof(fhd), fp);
  }

  for (uint64_t bidx = 0; bidx < block_num; bidx++) {
    fwrite(&src[bidx].tag, sizeof(src[bidx].tag), 1, fp);
    fwrite(&src[bidx].bytes, sizeof(src[bidx].bytes), 1, fp);

    if (src[bidx].bytes > 0) {
      fwrite(src[bidx].data, 1, src[bidx].bytes, fp);
    }
  }

  fclose(fp);
  return true;
}

void fractal_utils::binfile::remove_all_blocks() noexcept {
  for (auto &blk : this->blocks) {
    if (blk.data != nullptr)
      this->callback_free(blk.data);
    blk.data = nullptr;
  }

  this->blocks.clear();
}

fractal_utils::binfile::~binfile() { this->remove_all_blocks(); }

uint64_t get_file_size(FILE *fp_r) noexcept {
  const uint64_t idx = ftell(fp_r);

  fseek(fp_r, 0, SEEK_END);
  const uint64_t result = ftell(fp_r);

  fseek(fp_r, idx, SEEK_SET);

  return result;
}

bool fractal_utils::binfile::parse_from_file(const char *const filename,
                                             const bool offset_only) noexcept {

  if (filename == nullptr) {
    return false;
  }

  FILE *fp;
#ifdef _WIN32
  fopen_s(&fp, filename, "rb");
#else
  fp = fopen(filename, "rb");
#endif

  if (fp == nullptr) {
    return false;
  }

  // const uint64_t file_size = get_file_size(fp);

  {
    file_header fh;

    const int bytes = fread(&fh, 1, sizeof(fh), fp);
    if (bytes != sizeof(fh)) {
      return false;
    }

    if (!fh.is_valid()) {
      return false;
    }
  }

  while (true) {

    if (feof(fp)) {
      break;
    }

    int error_code = 0;
    int bytes = 0;

    data_block blk;

    bytes += fread(&blk.tag, sizeof(blk.tag), 1, fp);
    bytes += fread(&blk.bytes, sizeof(blk.bytes), 1, fp);

    if (bytes != sizeof(int64_t) + sizeof(uint64_t)) {
      return false;
    }
    blk.file_offset = ftell(fp);

    if (blk.bytes <= 0) {
      blk.data = nullptr;

      this->blocks.emplace_back(blk);
      continue;
    }

    if (offset_only) {
      error_code = fseek(fp, blk.bytes, SEEK_CUR);
      blk.data = nullptr;

      if (error_code) {
        printf(
            "\nError : function fractal_utils::binfile::parse_from_file failed "
            "to parse file %s. function fseek failed with error code %i.\n",
            filename, error_code);
        return false;
      }
    } else {
      blk.data = this->callback_malloc(blk.bytes);

      if (blk.data == nullptr) {

        printf(
            "\nError : function fractal_utils::binfile::parse_from_file failed "
            "to parse file %s. memory allocation function failed.\n",
            filename);
        return false;
      }

      const uint64_t __bytes = fread(blk.data, 1, blk.bytes, fp);

      if (__bytes != blk.bytes) {

        printf(
            "\nError : function fractal_utils::binfile::parse_from_file failed "
            "to parse file %s. fread met end of file.\n",
            filename);
        return false;
      }
    }

    this->blocks.emplace_back(blk);
  }

  return true;
}