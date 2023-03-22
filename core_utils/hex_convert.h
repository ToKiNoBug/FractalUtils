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

#ifndef FRACTAL_UTILS_CORE_UTILS_HEX_CONVERT_H
#define FRACTAL_UTILS_CORE_UTILS_HEX_CONVERT_H

#include <optional>
#include <string_view>

namespace fractal_utils {

std::optional<size_t> hex_2_bin(const char *src_beg, const char *src_end,
                                void *dst, size_t dst_capacity) noexcept;

std::optional<size_t> hex_2_bin(std::string_view, void *dst,
                                size_t dst_capacity) noexcept;

std::optional<size_t> bin_2_hex(const void *src, size_t src_bytes, char *dst,
                                size_t dst_capacity,
                                bool starts_with_0x) noexcept;
} // namespace fractal_utils

#endif // FRACTAL_UTILS_CORE_UTILS_HEX_CONVERT_H