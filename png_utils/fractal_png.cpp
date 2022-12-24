#include "png_utils.h"

#include <png.h>
#include <stdio.h>

struct write_struct {
  png_struct *png;
  png_info *info;
  FILE *fp;
  bool success;
};

write_struct create_write_struct(const char *const filename) noexcept {
  write_struct w;

  FILE *fp;

#ifdef _WIN32
  fopen_s(&fp, filename, "wb");
#else
  fp = fopen(filename, "wb");
#endif

  if (fp == NULL) {
    printf("\nError : function write_png failed. fopen failed.\n");
    w.success = false;
    return w;
  }

  w.fp = fp;

  png_struct *png =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png == NULL) {
    printf("\nError : function write_png failed. libpng failed to create "
           "png_struct.\n");
    w.success = false;
    return w;
  }

  png_info *png_info = png_create_info_struct(png);

  if (png_info == NULL) {
    printf("\nError : function write_png failed. libpng failed to create "
           "png_info struct.\n");
    png_destroy_write_struct(&png, &png_info);
    w.success = false;
    return w;
  }

  w.png = png;
  w.info = png_info;
  w.success = true;

  return w;
}

void destroy_write_struct(write_struct *const w) {
  if (w->info != NULL) {
    png_destroy_info_struct(w->png, &w->info);
    // w->info = NULL;
  }

  if (w->png != NULL) {
    png_destroy_write_struct(&w->png, &w->info);
  }

  if (w->fp != NULL) {
    fclose(w->fp);
    w->fp = NULL;
  }
}

bool fractal_utils::write_png(const char *const filename, const color_space cs,
                              const fractal_map &map) noexcept {
  using namespace fractal_utils;
  const bool is_ok = uint32_t(cs) == map.element_bytes;

  if (!is_ok) {
    printf("\nError : function write_png failed. The given color space is "
           "u8c%i, but the size of element is %u.\n",
           int(cs), map.element_bytes);
    return false;
  }

  write_struct wt = create_write_struct(filename);

  if (!wt.success) {
    return false;
  }

  png_init_io(wt.png, wt.fp);

  switch (cs) {
  case color_space::u8c1:
    png_set_IHDR(wt.png, wt.info, map.cols, map.rows, 8, PNG_COLOR_TYPE_GRAY,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);
    break;
  case color_space::u8c3:
    png_set_IHDR(wt.png, wt.info, map.cols, map.rows, 8, PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);
    break;
  case color_space::u8c4:
    png_set_IHDR(wt.png, wt.info, map.cols, map.rows, 8, PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);
    png_set_swap_alpha(wt.png);
    break;
  }

  png_write_info(wt.png, wt.info);

  for (uint64_t r = 0; r < map.rows; r++) {
    png_write_row(wt.png, reinterpret_cast<const uint8_t *>(map.data) +
                              r * map.cols * map.element_bytes);
  }

  png_write_end(wt.png, wt.info);

  destroy_write_struct(&wt);

  return true;
}

bool fractal_utils::write_png(const char *const filename, const color_space cs,
                              const void *const *const row_ptrs,
                              const uint64_t rows,
                              const uint64_t cols) noexcept {
  using namespace fractal_utils;

  write_struct wt = create_write_struct(filename);

  if (!wt.success) {
    return false;
  }

  png_init_io(wt.png, wt.fp);

  switch (cs) {
  case color_space::u8c1:
    png_set_IHDR(wt.png, wt.info, cols, rows, 8, PNG_COLOR_TYPE_GRAY,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);
    break;
  case color_space::u8c3:
    png_set_IHDR(wt.png, wt.info, cols, rows, 8, PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);
    break;
  case color_space::u8c4:
    png_set_IHDR(wt.png, wt.info, cols, rows, 8, PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);
    png_set_swap_alpha(wt.png);
    break;
  }

  png_write_info(wt.png, wt.info);

  for (uint64_t r = 0; r < rows; r++) {
    png_write_row(wt.png, reinterpret_cast<const uint8_t *>(row_ptrs[r]));
  }

  png_write_end(wt.png, wt.info);

  destroy_write_struct(&wt);

  return true;
}