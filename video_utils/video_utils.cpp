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

#include "video_utils.h"
#include "png_utils.h"
#include <fmt/format.h>
#include <filesystem>
#include <omp.h>
#include <atomic>
#include <mutex>
#include <fstream>

namespace stdfs = std::filesystem;
using namespace fractal_utils;

std::string common_info_base::size_expression_4ffmpeg() const noexcept {
  return fmt::format("{}x{}", this->cols(), this->rows());
}

std::string video_task_base::video_config::encode_expr_4ffmpeg()
    const noexcept {
  return fmt::format("-c:v {} {}", this->encoder, this->encoder_flags);
}

std::optional<full_task> video_executor_base::load_task(
    std::string &err) const noexcept {
  err.clear();
  auto ci = this->load_common_info(err);
  if (ci == nullptr || !err.empty()) {
    err = fmt::format("Failed to load common info. Details: {}", err);
    return std::nullopt;
  }

  auto ct = this->load_compute_task(err);
  if (ct == nullptr || !err.empty()) {
    err = fmt::format("Failed to load compute task. Details: {}", err);
    return std::nullopt;
  }

  auto rt = this->load_render_task(err);
  if (ct == nullptr || !err.empty()) {
    err = fmt::format("Failed to load render task. Details: {}", err);
    return std::nullopt;
  }

  auto vt = this->load_video_task(err);
  if (ct == nullptr || !err.empty()) {
    err = fmt::format("Failed to load video task. Details: {}", err);

    return std::nullopt;
  }
  return full_task{std::move(ci), std::move(ct), std::move(rt), std::move(vt)};
}

bool video_executor_base::check_archive(
    std::string_view filename, std::span<uint8_t> buffer,
    std::any *return_archive) const noexcept {
  std::any ar;
  std::string err = this->load_archive(filename, buffer, ar);
  if (!ar.has_value() || !err.empty()) {
    return false;
  }

  if (!this->error_of_archive(filename, ar).empty()) {
    return false;
  }

  if (return_archive != nullptr) {
    *return_archive = std::move(ar);
  }
  return true;
}

void video_executor_base::archive_filename(int archive_index,
                                           std::string &ret) const noexcept {
  ret.clear();
  const auto &ct = this->m_task.compute;
  fmt::format_to(std::back_inserter(ret), "{}archive{}{}.{}",
                 ct->archive_prefix, archive_index, ct->archive_suffix,
                 ct->archive_extension);
}

std::string video_executor_base::archive_filename(
    int archive_index) const noexcept {
  std::string ret;
  this->archive_filename(archive_index, ret);
  return ret;
}

void video_executor_base::image_filename(int archive_index, int image_idx,
                                         std::string &ret) const noexcept {
  const auto &rt = this->m_task.render;
  ret.clear();

  if (image_idx < rt->image_per_frame) {
    fmt::format_to(std::back_inserter(ret), "{}image{:06}-{}{}.{}",
                   rt->image_prefix, archive_index, image_idx, rt->image_suffix,
                   rt->image_extension);
  } else {
    fmt::format_to(std::back_inserter(ret), "{}image-extra{:06}{}{}.{}",
                   rt->image_prefix, archive_index, image_idx, rt->image_suffix,
                   rt->image_extension);
  }
}

void video_executor_base::image_filename_4ffmpeg(
    int archive_index, bool is_extra, std::string &ret) const noexcept {
  ret.clear();
  const auto &rt = this->m_task.render;

  if (!is_extra) {
    fmt::format_to(std::back_inserter(ret), "{}image%06d-{}{}.{}",
                   rt->image_prefix, archive_index, rt->image_suffix,
                   rt->image_extension);
  } else {
    fmt::format_to(std::back_inserter(ret), "{}image-extra%06d{}{}.{}",
                   rt->image_prefix, archive_index, rt->image_suffix,
                   rt->image_extension);
  }
}

std::string video_executor_base::image_filename(int archive_index,
                                                int image_idx) const noexcept {
  std::string ret;
  this->image_filename(archive_index, image_idx, ret);
  return ret;
}

void video_executor_base::video_temp_filename(int archive_index, bool is_extra,
                                              std::string &ret) const noexcept {
  ret.clear();
  const auto &vt = this->m_task.video;
  if (!is_extra) {
    fmt::format_to(std::back_inserter(ret), "{}temp{:06}{}.{}",
                   vt->temp_config.video_prefix, archive_index,
                   vt->temp_config.video_suffix, vt->temp_config.extension);
  } else {
    fmt::format_to(std::back_inserter(ret), "{}temp-extra{:06}{}.{}",
                   vt->temp_config.video_prefix, archive_index,
                   vt->temp_config.video_suffix, vt->temp_config.extension);
  }
}

std::string video_executor_base::video_temp_filename(
    int archive_index, bool is_extra) const noexcept {
  std::string ret;
  this->video_temp_filename(archive_index, is_extra, ret);
  return ret;
}

void video_executor_base::video_second_temp_filename(
    int archive_index, std::string &ret) const noexcept {
  ret.clear();
  const auto &vt = *this->m_task.video;
  fmt::format_to(std::back_inserter(ret), "{}second-temp{:06}{}.{}",
                 vt.temp_config.video_prefix, archive_index,
                 vt.temp_config.video_suffix, vt.temp_config.extension);
}

std::string video_executor_base::video_second_temp_filename(
    int archive_index) const noexcept {
  std::string ret;
  ret.reserve(1024);
  this->video_second_temp_filename(archive_index, ret);
  return ret;
}

void video_executor_base::product_filename(std::string &ret) const noexcept {
  ret.clear();
  const auto &vt = this->m_task.video;
  const auto &pc = vt->product_config;
  fmt::format_to(std::back_inserter(ret), "{}{}{}.{}", pc.video_prefix,
                 vt->product_name, pc.video_suffix, pc.extension);
}

std::string video_executor_base::product_filename() const noexcept {
  std::string ret;
  this->product_filename(ret);
  return ret;
}

bool can_be_regular_file(const stdfs::path &filename) noexcept {
  try {
    if (!stdfs::exists(filename)) {
      return false;
    }
    if (stdfs::is_regular_file(filename)) {
      return true;
    }
    if (!stdfs::is_symlink(filename)) {
      return false;
    }

    return can_be_regular_file(stdfs::read_symlink(filename));
  } catch (...) {
    return false;
  }
}

bool fractal_utils::can_be_regular_file(std::string_view filename) noexcept {
  return ::can_be_regular_file(stdfs::path{filename});
}

bool create_required_dirs(const stdfs::path &filename) noexcept {
  const auto parent_path = filename.parent_path();
  if (stdfs::is_directory(parent_path)) {
    return true;
  }
  std::error_code err;
  if (stdfs::create_directories(parent_path, err)) {
    return true;
  }
  return false;
}

bool video_executor_base::load_task() & noexcept {
  std::string err;
  auto temp = this->load_task(err);
  if (!temp.has_value() || err.empty()) {
    return false;
  }
  this->m_task = std::move(temp.value());
  return true;
}

std::vector<uint8_t> video_executor_base::compute_task_status(
    std::string &filename, std::any &archive,
    std::span<uint8_t> buffer) const noexcept {
  const auto &common = *this->m_task.common;
  std::vector<uint8_t> task_lut;
  task_lut.resize(common.archive_num);
  std::fill(task_lut.begin(), task_lut.end(), false);

  filename.reserve(1024);
  for (int aidx = 0; aidx < common.archive_num; aidx++) {
    this->archive_filename(aidx, filename);

    if (!create_required_dirs(filename)) {
      fmt::print(
          "Warning: Failed to create required directories for {}, the "
          "computation may fail.\n",
          filename);
    }

    if (!can_be_regular_file(filename)) {
      continue;
    }

    if (this->check_archive(filename, buffer, nullptr)) {
      fmt::print(
          "{} exists but is found to be corrupted, it will be generated "
          "again.\n",
          filename);
      continue;
    }
    task_lut[aidx] = true;
  }

  return task_lut;
}

bool video_executor_base::run_compute() const noexcept {
  const auto &common = *this->m_task.common;
  const auto &ct = *this->m_task.compute;

  omp_set_num_threads(ct.threads);

  std::any archive;
  std::string filename;
  filename.reserve(1024);
  std::vector<uint8_t> load_archive_buffer;

  load_archive_buffer.resize(common.suggested_load_buffer_size());
  // 1 -> finished
  const auto task_lut =
      this->compute_task_status(filename, archive, load_archive_buffer);
  filename.clear();

  int finished_tasks{0};
  for (auto status : task_lut) {
    finished_tasks += bool(status);
  }
  const int already_finished_tasks = finished_tasks;

  std::unique_ptr<wind_base> current_wind{ct.start_window()->create_another()};

  for (int aidx = 0; aidx < common.archive_num; aidx++) {
    if (task_lut[aidx]) {
      continue;
    }
    this->archive_filename(aidx, filename);
    fmt::print(
        "[{} / {} : {}%] : computing {}\n", finished_tasks, common.archive_num,
        float(finished_tasks * 100) / float(common.archive_num), filename);

    ct.start_window()->copy_to(current_wind.get());
    current_wind->update_scale(common.ratio, aidx);

    this->compute(aidx, *current_wind, archive);

    auto err = this->save_archive(archive, filename);
    if (err.empty()) {
      finished_tasks++;
      continue;
    }

    fmt::print("Failed to generate {}, details: {}\n", filename, err);
    return false;

    // scale here
  }

  fmt::print("All tasks finished, {} archives generated in this run.\n",
             common.archive_num - already_finished_tasks);
  return true;
}

std::vector<video_executor_base::render_status>
video_executor_base::render_task_status() const noexcept {
  const auto &common = *this->m_task.common;
  const auto &ct = *this->m_task.compute;
  const auto &rt = *this->m_task.render;
  std::vector<render_status> ret;
  ret.resize(common.archive_num);
  // std::fill(ret.begin(), ret.end(), render_status::not_rendered);

#pragma omp parallel for default(none) shared(common, ct, rt, ret) \
    schedule(dynamic)
  for (int aidx = 0; aidx < common.archive_num; aidx++) {
    std::string buffer;
    buffer.resize(1024);
    int existing_image_count{0};
    for (int iidx = 0; iidx < rt.image_count(); iidx++) {
      this->image_filename(aidx, iidx, buffer);
      if (can_be_regular_file(buffer)) {
        existing_image_count++;
      }
    }

    if (existing_image_count == 0) {
      ret[aidx] = render_status::not_rendered;
      continue;
    }
    if (existing_image_count == rt.image_count()) {
      ret[aidx] = render_status::all_rendered;
      continue;
    }
    ret[aidx] = render_status::partly_rendered;
  }

  return ret;
}

bool video_executor_base::run_render() const noexcept {
  const auto &common = *this->m_task.common;
  const auto &ct = *this->m_task.compute;
  const auto &rt = *this->m_task.render;

  const auto render_status = this->render_task_status();
  assert(render_status.size() == common.archive_num);

  std::atomic<int> fully_rendered_archive_count{0};
  {
    std::string filename;
    filename.reserve(1024);
    std::vector<uint8_t> buffer_archive;
    buffer_archive.reserve(common.suggested_load_buffer_size());
    for (int aidx = 0; aidx < render_status.size(); aidx++) {
      const auto status = render_status[aidx];
      if (status == render_status::all_rendered) {
        fully_rendered_archive_count++;
        continue;
      }
      this->archive_filename(aidx, filename);
      if (!can_be_regular_file(filename)) {
        fmt::print(
            "Images of {} are not fully rendered, but this archive file is "
            "missing.\n",
            filename);
        return false;
      }
      if (!this->check_archive(filename, buffer_archive, nullptr)) {
        fmt::print(
            "Images of {} are not fully rendered, but this archive file is "
            "corrupted.\n",
            filename);
        return false;
      }
    }
  }

  const int already_rendered_archives = fully_rendered_archive_count;

  std::mutex lock;

  omp_set_num_threads(rt.threads);

#pragma omp parallel for default(shared)                                      \
    shared(common, ct, rt, render_status, lock, fully_rendered_archive_count) \
    schedule(dynamic)
  for (int aidx = 0; aidx < common.archive_num; aidx++) {
    if (render_status[aidx] == render_status::all_rendered) {
      continue;
    }

    thread_local std::any archive;
    thread_local std::vector<uint8_t> buffer;
    thread_local std::string filename;
    thread_local unique_map image_u8c3{common.rows(), common.cols(), 3};
    thread_local std::vector<const void *> row_ptrs;
    thread_local std::unique_ptr<render_resource_base> render_resource =
        this->create_render_resource();

    buffer.resize(common.suggested_load_buffer_size());
    filename.reserve(1024);
    row_ptrs.reserve(common.cols());

    this->archive_filename(aidx, filename);

    if (lock.try_lock()) {
      fmt::print(
          "[{} / {} : {}%] : Rendering {}\n", int(fully_rendered_archive_count),
          common.archive_num,
          100.0f * int(fully_rendered_archive_count) / common.archive_num,
          filename);
      lock.unlock();
    }

    {
      auto err = this->load_archive(filename, buffer, archive);
      if (!archive.has_value() || !err.empty()) {
        std::lock_guard<std::mutex> lkgd{lock};
        fmt::print("Fatal : failed to load {}, detail: {}.\n", filename, err);
        continue;
      }
    }

    const bool render_once = rt.render_once;
    if (render_once) {
      auto err =
          this->render(archive, aidx, 0, image_u8c3, render_resource.get());
      if (!err.empty()) {
        std::lock_guard<std::mutex> lkgd{lock};
        fmt::print(
            "Fatal: failed to render {} with image_idx = {}, render_once = {}, "
            "detail: {}\n",
            filename, 0, render_once, err);
        continue;
      }
    }

    std::string image_filename;
    image_filename.reserve(1024);

    bool fail_to_render{false};
    for (int iidx = 0; iidx < rt.image_count(); iidx++) {
      this->image_filename(aidx, iidx, image_filename);

      if (!render_once) {
        auto err =
            this->render(archive, aidx, 0, image_u8c3, render_resource.get());
        if (!err.empty()) {
          std::lock_guard<std::mutex> lkgd{lock};
          fmt::print(
              "Fatal: failed to render {} with image_idx = {}, render_once = "
              "{}, "
              "detail: {}\n",
              filename, 0, render_once, err);
          fail_to_render = true;
          break;
        }
      }

      const int skip_r =
          skip_rows(common.rows(), common.ratio, rt.image_per_frame, iidx);
      const int skip_c =
          skip_cols(common.cols(), common.ratio, rt.image_per_frame, iidx);

      if (!write_png_skipped(image_filename.c_str(), color_space::u8c3,
                             image_u8c3, skip_r, skip_c, row_ptrs)) {
        std::lock_guard<std::mutex> lkgd{lock};
        fmt::print(
            "Fatal: failed to save {} with archive filename= {} with image_idx "
            "= {}, render_once = {}\n",
            image_filename, filename, 0, render_once);
        fail_to_render = true;
        break;
      }
    }

    if (fail_to_render) {
      continue;
    }

    fully_rendered_archive_count++;
  }

  if (fully_rendered_archive_count != common.archive_num) {
    fmt::print("{} archives failed to be rendered.\n",
               common.archive_num - fully_rendered_archive_count);
    return false;
  }
  return true;
}
