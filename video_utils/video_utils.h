
#ifndef FRACTALUTILS_VIDEOUTILS_VIDEOUTILS_H
#define FRACTALUTILS_VIDEOUTILS_VIDEOUTILS_H

#include "core_utils.h"
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <optional>
#include <any>

namespace fractal_utils {

struct common_info_base {
  size_t rows;
  size_t cols;
  int archive_num;
  double ratio;
};

struct compute_task_base {
  std::unique_ptr<wind_base> start_window{nullptr};
  std::string archive_prefix;
  std::string archive_suffix;
  std::string archive_extension{"bin"};
  int threads;
};

struct render_task_base {
  int image_per_frame;
  int extra_image_num;
  int threads;
  std::string image_prefix;
  std::string image_suffix;
  std::string image_extension{"png"};
};

struct video_task_base {
  struct video_config {
    std::string extension{"mp4"};
    std::string encoder{"x264"};
    std::string encoder_flags;
    std::string video_prefix;
    std::string video_suffix;
  };

  video_config temp_config;
  video_config product_config;
  std::string product_name;
  std::string ffmpeg_exe;
  int threads;
};

struct full_task {
  std::unique_ptr<common_info_base> common;
  std::unique_ptr<compute_task_base> compute;
  std::unique_ptr<render_task_base> render;
  std::unique_ptr<video_task_base> video;
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

  inline void set_task(full_task &&src) noexcept {
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

  virtual void video_temp_filename(int archive_index, bool is_extra,
                                   std::string &ret) const noexcept;
  [[nodiscard]] std::string video_temp_filename(int archive_index,
                                                bool is_extra) const noexcept;

  virtual void product_filename(std::string &ret) const noexcept;
  [[nodiscard]] std::string product_filename() const noexcept;

  // run operations

  [[nodiscard]] bool load_task() noexcept;

  [[nodiscard]] virtual bool run_compute() const noexcept;

  [[nodiscard]] virtual bool run_render() const noexcept;

  [[nodiscard]] virtual bool make_video() const noexcept;

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

  [[nodiscard]] virtual err_info_t save_archive(
      const std::any &, std::string_view filename) const noexcept = 0;

  [[nodiscard]] virtual bool check_archive(
      std::string_view filename,
      std::any *return_archive_nullable) const noexcept;

  [[nodiscard]] virtual std::any load_archive(
      std::string_view filename, std::string &err) const noexcept = 0;

  [[nodiscard]] virtual err_info_t render_archive(
      const std::any &archive, int archive_index, int image_idx,
      map_view map) const noexcept = 0;
};

}  // namespace fractal_utils

#endif  // FRACTALUTILS_VIDEOUTILS_VIDEOUTILS_H
