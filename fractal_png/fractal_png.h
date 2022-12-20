#ifndef FRACTALUTILS_FRACTAL_PNG_H
#define FRACTALUTILS_FRACTAL_PNG_H

#include <fractal_map.h>

namespace fractal_utils {

enum class color_space : uint8_t { u8c1 = 1, u8c3 = 3, u8c4 = 4 };

bool write_png(const char *const filename, const color_space,
               const fractal_map &map) noexcept;

bool write_png(const char *const filename, const color_space,
               const void *const *const row_ptrs, const uint64_t rows,
               const uint64_t cols) noexcept;
} // namespace fractal_utils

#endif // FRACTALUTILS_FRACTAL_PNG_H