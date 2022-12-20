#include "fractal_binfile.h"

#include <corecrt_malloc.h>
#include <stdio.h>

bool generate_file(const char *const filename);

bool parse_file(const char *const filename);

int main() {
  if (!generate_file("test.bin")) {
    return 1;
  }

  parse_file("test.bin");

  return 0;
}

bool generate_file(const char *const filename) {

  std::vector<fractal_utils::data_block> blocks;
  blocks.resize(2);

  blocks[0].tag = 1;
  blocks[0].bytes = 64;
  blocks[0].data = malloc(64);
  memset(blocks[0].data, 2, 64);

  blocks[1].tag = 114514;
  blocks[1].bytes = 95;
  blocks[1].data = malloc(95);
  memset(blocks[1].data, 6, 95);

  const bool success = fractal_utils::serialize_to_file(
      blocks.data(), blocks.size(), true, filename);

  for (auto &i : blocks) {
    if (i.bytes > 0)
      free(i.data);
    i.data = nullptr;
  }

  if (!success) {
    printf("Failed\n");
    return false;
  } else {
    printf("Success\n");
    return true;
  }
}

bool parse_file(const char *const filename) {
  fractal_utils::binfile file;

  printf("parsing binfile...\n");
  const bool success = file.parse_from_file(filename);

  if (!success) {
    printf("parse_file failed\n");
    return false;
  }

  printf("parse_file succeeded\n");

  for (int i = 0; i < file.blocks.size(); i++) {

    printf("block %i : tag = %lli, %llu bytes, offset = %llu\n", i,
           file.blocks[i].tag, file.blocks[i].bytes,
           file.blocks[i].file_offset);
  }

  return true;
}