/*
Copyright © 2022-2023  TokiNoBug
This file is part of FractalUtils.

FractalUtils is free software: you can redistribute it and/or modify
                                                                    it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

                                        FractalUtils is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with FractalUtils.  If not, see <https://www.gnu.org/licenses/>.

   Contact with me:
   github:https://github.com/ToKiNoBug
*/

#ifndef FRACTALUTILS_VIDEOUTILS_VIDEOUTILS_H
#define FRACTALUTILS_VIDEOUTILS_VIDEOUTILS_H

#include "core_utils.h"
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <optional>
#include <any>
#include <variant>

namespace fractal_utils {

class common_info_base {
 public:
  virtual ~common_info_base() = default;
  [[nodiscard]] virtual size_t rows() const noexcept = 0;
  [[nodiscard]] virtual size_t cols() const noexcept = 0;
  // size_t rows;
  // size_t cols;
  int archive_num{-1};
  double ratio{2};

  [[nodiscard]] virtual size_t suggested_load_buffer_size() const noexcept {
    return 1 << 20;
  }

  [[nodiscard]] std::string size_expression_4ffmpeg() const noexcept;
};

class compute_task_base {
 public:
  virtual ~compute_task_base() = default;
  // std::unique_ptr<wind_base> start_window{nullptr};
  [[nodiscard]] virtual wind_base *start_window() noexcept = 0;
  [[nodiscard]] virtual const wind_base *start_window() const noexcept = 0;
  std::string archive_prefix;
  std::string archive_suffix;
  std::string archive_extension{"bin"};
  int threads;
};

class render_task_base {
 public:
  virtual ~render_task_base() = default;
  int image_per_frame;
  int extra_image_num;
  int threads{4};
  bool render_once;
  std::string image_prefix;
  std::string image_suffix;
  std::string image_extension{"png"};

  [[nodiscard]] inline int image_count() const noexcept {
    return this->image_per_frame + this->extra_image_num;
  }
};

class video_task_base {
 public:
  virtual ~video_task_base() = default;

  struct video_config {
    std::string extension{"mp4"};
    std::string encoder{"x264"};
    std::string encoder_flags;
    std::string video_prefix;
    std::string video_suffix;

    [[nodiscard]] std::string encode_expr_4ffmpeg() const noexcept;
  };

  video_config temp_config;
  video_config product_config;
  std::string product_name;
  std::string ffmpeg_exe;
  int threads;
  bool prefer_symlink{false};
};

struct full_task {
  std::unique_ptr<common_info_base> common;
  std::unique_ptr<compute_task_base> compute;
  std::unique_ptr<render_task_base> render;
  std::unique_ptr<video_task_base> video;
};

class render_resource_base {
 public:
  virtual ~render_resource_base() = default;
};

class video_executor_base {
 protected:
  full_task m_task;

 public:
  video_executor_base() = default;
  video_executor_base(video_executor_base &&another) = default;
  virtual ~video_executor_base() = default;

  using err_info_t = std::string;

  [[nodiscard]] full_task &task() noexcept { return this->m_task; }
  [[nodiscard]] const full_task &task() const noexcept { return this->m_task; }

  virtual void set_task(full_task &&src) & noexcept {
    this->m_task = std::move(src);
  }

  // filename

  virtual void archive_filename(int archive_index,
                                std::string &ret) const noexcept;
  [[nodiscard]] std::string archive_filename(int archive_index) const noexcept;

  virtual void image_filename(int archive_index, int image_idx,
                              std::string &ret) const noexcept;
  [[nodiscard]] std::string image_filename(int archive_index,
                                           int image_idx) const noexcept;

  virtual void image_filename_4ffmpeg(int archive_index, bool is_extra,
                                      std::string &ret) const noexcept;

  virtual void video_temp_filename(int archive_index, bool is_extra,
                                   std::string &ret) const noexcept;
  [[nodiscard]] std::string video_temp_filename(int archive_index,
                                                bool is_extra) const noexcept;

  virtual void video_second_temp_filename(int archive_index,
                                          std::string &ret) const noexcept;
  [[nodiscard]] std::string video_second_temp_filename(
      int archive_index) const noexcept;

  virtual void product_filename(std::string &ret) const noexcept;
  [[nodiscard]] std::string product_filename() const noexcept;

  // run operations

  [[nodiscard]] virtual bool load_task() & noexcept;

  // 1 -> finished
  [[nodiscard]] std::vector<uint8_t> compute_task_status() const noexcept {
    std::string buf;
    buf.reserve(1024);
    std::any buf_archive;
    return this->compute_task_status(buf, buf_archive, {});
  }
  [[nodiscard]] virtual std::vector<uint8_t> compute_task_status(
      std::string &buf_filename, std::any &buf_archive,
      std::span<uint8_t> buffer) const noexcept;

  enum class render_status : uint8_t {
    not_rendered,
    partly_rendered,
    all_rendered
  };
  [[nodiscard]] virtual std::vector<render_status> render_task_status()
      const noexcept;

  [[nodiscard]] virtual bool run_compute() const noexcept;

  [[nodiscard]] virtual bool run_render() const noexcept;

  [[nodiscard]] virtual bool make_video(bool dry_run) const noexcept;

 protected:
  // load functions
  [[nodiscard]] virtual std::optional<full_task> load_task(
      std::string &err) const noexcept;

  [[nodiscard]] virtual std::unique_ptr<common_info_base> load_common_info(
      std::string &err) const noexcept = 0;
  [[nodiscard]] virtual std::unique_ptr<compute_task_base> load_compute_task(
      std::string &err) const noexcept = 0;
  [[nodiscard]] virtual std::unique_ptr<render_task_base> load_render_task(
      std::string &err) const noexcept = 0;
  [[nodiscard]] virtual std::unique_ptr<video_task_base> load_video_task(
      std::string &err) const noexcept = 0;

  // compute and render

  virtual void compute(int archive_idx, const wind_base &window,
                       std::any &ret) const noexcept = 0;

  [[nodiscard]] virtual std::unique_ptr<render_resource_base>
  create_render_resource() const noexcept {
    return nullptr;
  };

  [[nodiscard]] virtual std::string render(
      const std::any &archive, int archive_index, int image_idx,
      map_view image_u8c3, render_resource_base *resource) const noexcept {
    return this->render_with_skip(archive, archive_index, image_idx, 0, 0,
                                  image_u8c3, resource);
  }

  [[nodiscard]] virtual std::string render_with_skip(
      const std::any &archive, int archive_index, int image_idx, int skip_rows,
      int skip_cols, map_view image_u8c3,
      render_resource_base *resource) const noexcept = 0;

  [[nodiscard]] virtual err_info_t save_archive(
      const std::any &, std::string_view filename) const noexcept = 0;

  [[nodiscard]] inline bool check_archive(
      std::string_view filename,
      std::any *return_archive_nullable) const noexcept {
    return this->check_archive(filename, {}, return_archive_nullable);
  }
  [[nodiscard]] virtual bool check_archive(
      std::string_view filename, std::span<uint8_t> buffer,
      std::any *return_archive_nullable) const noexcept;

  [[nodiscard]] virtual err_info_t error_of_archive(
      std::string_view filename, std::any &archive) const noexcept = 0;

  [[nodiscard]] inline std::string load_archive(
      std::string_view filename, std::any &archive) const noexcept {
    return this->load_archive(filename, {}, archive);
  }
  [[nodiscard]] virtual std::string load_archive(
      std::string_view filename, std::span<uint8_t> buffer,
      std::any &archive) const noexcept = 0;

  [[nodiscard]] virtual bool make_temp_video(int aidx,
                                             bool dry_run) const noexcept;
  [[nodiscard]] virtual bool make_temp_extra_video(int aidx,
                                                   bool dry_run) const noexcept;
  [[nodiscard]] virtual bool make_second_temp_video(
      int aidx, bool dry_run) const noexcept;
  [[nodiscard]] virtual bool make_second_temp_list_txt(
      std::string_view txt_filename,
      std::span<const std::string> concate_sources,
      bool dry_run) const noexcept;
  [[nodiscard]] virtual bool make_product_video(std::string_view txt_filename,
                                                bool dry_run) const noexcept;
};

[[nodiscard]] int run_command(std::string_view command, bool dry_run) noexcept;

[[nodiscard]] bool can_be_regular_file(std::string_view filename) noexcept;
[[nodiscard]] bool create_required_dirs(std::string_view filename) noexcept;
}  // namespace fractal_utils

#endif  // FRACTALUTILS_VIDEOUTILS_VIDEOUTILS_H
