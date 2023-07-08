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

#include <cstring>
#include <fmt/format.h>
#include <stdio.h>

#include "binary_archive.h"

bool generate_file(const char *const filename);

bool parse_file(const char *const filename);

int main() {
  if (!generate_file("test.archive")) {
    return 1;
  }
  if (!parse_file("test.archive")) {
    return 1;
  }

  return 0;
}

bool generate_file(const char *const filename) {
  fractal_utils::binary_archive archive;
  {
    using variant_t = fractal_utils::data_segment::variant_t;
    std::vector<uint8_t> data;

    data.resize(64);
    std::fill(data.begin(), data.end(), 2);
    archive.segments().emplace_back(fractal_utils::data_segment{1, data});

    data.resize(1919810);
    std::fill(data.begin(), data.end(), 255);
    archive.segments().emplace_back(
        fractal_utils::data_segment{1, std::move(data)});
  }

  const auto err = archive.save(filename);

  if (!err.empty()) {
    fmt::print("Failed. Detail: {}\n", err);
    return false;
  } else {
    printf("Success\n");
    return true;
  }
}

bool parse_file(const char *const filename) {
  fractal_utils::binary_archive archive;

  printf("parsing binfile...\n");

  std::vector<uint8_t> buffer;
  buffer.resize(0x1000000);
  size_t buffer_used_bytes{0};
  const auto err = archive.load(filename, buffer, &buffer_used_bytes);

  if (!err.empty()) {
    fmt::print("parse_file failed, detail: {}\n", err);
    return false;
  }

  fmt::print("parse_file succeeded, buffer_used_bytes = {}\n",
             buffer_used_bytes);

  for (int i = 0; i < archive.segments().size(); i++) {
    auto &seg = archive.segments()[i];
    fmt::print("block {} : tag = {}, {} bytes\n", i, seg.tag(), seg.bytes());
  }

  return true;
}