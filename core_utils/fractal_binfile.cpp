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

#include "fractal_binfile.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __GNUC__
#include <cstring>
#endif

using namespace fractal_utils;
constexpr char __cmp[] = {'A', 'r', 'm', 'a', 'g', 'e', 'd', 'd',
                          'o', 'n', 6,   6,   6,   42,  30,  0};
fractal_utils::file_header::file_header() {
#ifdef __GNUC__
  __builtin_memset(this->data, 0, sizeof(file_header));
  __builtin_memcpy(this->data, __cmp, sizeof(__cmp));
#else
  memset(this->data, 0, sizeof(file_header));
  memcpy(this->data, __cmp, sizeof(__cmp));
#endif
}

bool fractal_utils::file_header::is_valid() const noexcept {
  for (int idx = 0; idx < sizeof(__cmp) / sizeof(__cmp[0]); idx++) {
    if (this->data[idx] != __cmp[idx]) {
      return false;
    }
  }
  return true;
}

int8_t *fractal_utils::file_header::custom_part() noexcept {
  return this->data + sizeof(__cmp);
}

const int8_t *fractal_utils::file_header::custom_part() const noexcept {
  return this->data + sizeof(__cmp);
}

size_t fractal_utils::file_header::custom_part_len() noexcept {
  return 32 - sizeof(__cmp);
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

#ifdef __GNUC__
    __builtin_memcpy(dest, &h, sizeof(h));
#else
    memcpy(dest, &h, sizeof(h));
#endif
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

    if (!fhp->is_valid()) return false;
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
                                      const file_header *header_nullable,
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

  if (header_nullable != nullptr) {
    fwrite(header_nullable, 1, sizeof(file_header), fp);
  }

  for (uint64_t bidx = 0; bidx < block_num; bidx++) {
    if (!write_data_block(fp, src[bidx])) {
      printf("Failed to write data block %zu.", size_t(bidx));
      return false;
    }

    /*
    fwrite(&src[bidx].tag, sizeof(src[bidx].tag), 1, fp);
    fwrite(&src[bidx].bytes, sizeof(src[bidx].bytes), 1, fp);

    if (src[bidx].bytes > 0) {
      fwrite(src[bidx].data, 1, src[bidx].bytes, fp);
    }
    */
  }

  fclose(fp);
  return true;
}

void fractal_utils::binfile::remove_all_blocks() noexcept {
  for (auto &blk : this->blocks) {
    if (this->callback_free == nullptr) {
      continue;
    }
    if (!blk.has_ownership) {
      continue;
    }
    if (blk.data != nullptr) this->callback_free(blk.data);
    blk.data = nullptr;
  }

  this->blocks.clear();
}

bool fractal_utils::serialize_to_file(const data_block *const src,
                                      const uint64_t block_num,
                                      const bool write_header,
                                      const char *const filename) noexcept {
  file_header fhd;

  return fractal_utils::serialize_to_file(
      src, block_num, (write_header ? &fhd : nullptr), filename);
}

void fractal_utils::binfile::copy_from(const binfile &another) noexcept {
  this->remove_all_blocks();

  this->blocks.reserve(another.blocks.size());

  this->callback_malloc = another.callback_malloc;
  this->callback_free = another.callback_free;

  for (const auto &blk : another.blocks) {
    data_block blk_copy;
    blk_copy.tag = blk.tag;
    blk_copy.bytes = blk.bytes;
    blk_copy.file_offset = blk.file_offset;
    if (blk.data != nullptr) {
      assert(this->callback_malloc != nullptr);
      blk_copy.data = this->callback_malloc(blk.bytes);
      memcpy(blk_copy.data, blk.data, blk.bytes);
      blk_copy.has_ownership = true;
    } else {
      blk_copy.data = nullptr;
    }

    this->blocks.emplace_back(blk_copy);
  }
}

fractal_utils::binfile::binfile(const binfile &another) {
  this->copy_from(another);
}

fractal_utils::binfile::binfile(binfile &&another)
    : callback_malloc(another.callback_malloc),
      callback_free(another.callback_free),
      blocks(std::move(another.blocks)) {}

fractal_utils::binfile::~binfile() { this->remove_all_blocks(); }

const fractal_utils::binfile &fractal_utils::binfile::operator=(
    const binfile &another) noexcept {
  this->copy_from(another);
  return *this;
}

const fractal_utils::binfile &fractal_utils::binfile::operator=(
    binfile &&another) noexcept {
  this->remove_all_blocks();

  this->blocks = std::move(another.blocks);
  another.blocks.clear();

  this->callback_malloc = another.callback_malloc;
  this->callback_free = another.callback_free;
  return *this;
}

uint64_t get_file_size(FILE *fp_r) noexcept {
  const uint64_t idx = ftell(fp_r);

  fseek(fp_r, 0, SEEK_END);
  const uint64_t result = ftell(fp_r);

  fseek(fp_r, idx, SEEK_SET);

  return result;
}

fractal_utils::data_block *fractal_utils::binfile::find_block_single(
    int64_t tag) noexcept {
  for (auto &blk : this->blocks) {
    if (blk.tag == tag) {
      return &blk;
    }
  }

  return nullptr;
}

const fractal_utils::data_block *fractal_utils::binfile::find_block_single(
    int64_t tag) const noexcept {
  for (const auto &blk : this->blocks) {
    if (blk.tag == tag) {
      return &blk;
    }
  }
  return nullptr;
}

bool fractal_utils::binfile::save_to_file(
    const char *filename, const bool wirte_header) const noexcept {
  return ::fractal_utils::serialize_to_file(
      this->blocks.data(), this->blocks.size(),
      (wirte_header ? &this->header : nullptr), filename);
}

bool fractal_utils::binfile::parse_from_file(const char *const filename,
                                             const bool offset_only) noexcept {
  FILE *fp;
#ifdef _WIN32
  fopen_s(&fp, filename, "rb");
#else
  fp = fopen(filename, "rb");
#endif

  if (fp == nullptr) {
    printf(
        "\nError : function fractal_utils::binfile::parse_from_file failed "
        "to parse file %s : failed to open file stream.\n",
        filename);
    return false;
  }

  const uint64_t file_size = get_file_size(fp);

  {
    file_header fh;

    const int bytes = fread(&fh, 1, sizeof(fh), fp);
    if (bytes != sizeof(fh)) {
      printf(
          "\nError : function fractal_utils::binfile::parse_from_file failed "
          "to parse file %s : failed to read file header.\n",
          filename);
      return false;
    }

    if (!fh.is_valid()) {
      printf(
          "\nError : function fractal_utils::binfile::parse_from_file failed "
          "to parse file %s : file header is invalid.\n",
          filename);
      return false;
    }
    this->header = fh;
  }

  while (true) {
    if (ftell(fp) >= file_size) {
      break;
    }

    int error_code = 0;
    int bytes = 0;

    data_block blk;

    bytes += fread(&blk.tag, 1, sizeof(blk.tag), fp);
    bytes += fread(&blk.bytes, 1, sizeof(blk.bytes), fp);

    if (bytes != sizeof(int64_t) + sizeof(uint64_t)) {
      printf(
          "\nError : function fractal_utils::binfile::parse_from_file failed "
          "to parse file %s : tried to read a tag and lenght, but only read %i "
          "bytes.\n",
          filename, int(bytes));
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
      blk.has_ownership = true;

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
            "to parse file %s. fread meet end of file.\n",
            filename);
        return false;
      }
    }

    this->blocks.emplace_back(blk);
  }

  return true;
}

bool fractal_utils::write_data_block(::FILE *const file_ptr,
                                     const data_block &block) noexcept {
  // printf("\nCalling write_data_block.\n");
  size_t temp;
  temp = fwrite(&block.tag, 1, sizeof(block.tag), file_ptr);
  if (temp != sizeof(block.tag)) {
    printf(
        "\nError : function write_data_block failed when writing tag. fwrite "
        "should write %zu "
        "bytes but only wrote %zu bytes.\n",
        sizeof(block.tag), temp);
    return false;
  }
  temp = fwrite(&block.bytes, 1, sizeof(block.bytes), file_ptr);
  if (temp != sizeof(block.bytes)) {
    printf(
        "\nError : function write_data_block failed when writing length. "
        "fwrite should write %zu "
        "bytes but only wrote %zu bytes.\n",
        sizeof(block.bytes), temp);
    return false;
  }
  temp = fwrite(block.data, 1, block.bytes, file_ptr);
  if (temp != block.bytes) {
    printf(
        "\nError : function write_data_block failed when writing data. fwrite "
        "should write %zu "
        "bytes but only wrote %zu bytes.\n",
        block.bytes, temp);
    return false;
  }

  return true;
}