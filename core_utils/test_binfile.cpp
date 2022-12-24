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

#include <stdio.h>

#include <cstring>

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