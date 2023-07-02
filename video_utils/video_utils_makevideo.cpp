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
#include <fmt/format.h>
#include <filesystem>
#include <omp.h>
#include <atomic>
#include <mutex>
#include <fstream>

namespace stdfs = std::filesystem;
bool can_be_regular_file(const stdfs::path &filename) noexcept;

using namespace fractal_utils;

bool video_executor_base::make_video(bool dry_run) const noexcept {
  const auto &common = *this->m_task.common;
  const auto &ct = *this->m_task.compute;
  const auto &rt = *this->m_task.render;
  const auto &vt = *this->m_task.video;
  omp_set_num_threads(vt.threads);

  const auto render_status_vec = this->render_task_status();

  for (size_t aidx = 0; aidx < render_status_vec.size(); aidx++) {
    const auto rs = render_status_vec[aidx];
    if (rs != render_status::all_rendered) {
      fmt::print("One or more images of {} is not rendered.\n",
                 this->archive_filename(aidx));
      return false;
    }
  }

  std::atomic<int> finished_count{0};
  std::atomic<int> failed_count{0};
  std::mutex lock;

  // make fist temp videos
#pragma omp parallel for schedule(dynamic) default(none) \
    shared(common, ct, rt, vt, finished_count, failed_count, dry_run, lock)
  for (int aidx = 0; aidx < common.archive_num; aidx++) {
    if (!this->make_temp_video(aidx, dry_run)) {
      std::lock_guard<std::mutex> lkgd{lock};
      fmt::print("Failed to make temp video for archive {}.\n", aidx);
      failed_count++;
      continue;
    }
    finished_count++;

    // the extra video of last archive is useless
    if (aidx == common.archive_num - 1) {
      continue;
    }

    if (!this->make_temp_extra_video(aidx, dry_run)) {
      std::lock_guard<std::mutex> lkgd{lock};
      fmt::print("Failed to make extra temp video for archive {}.\n", aidx);
      failed_count++;
      continue;
    }
    finished_count++;
  }

  fmt::print(
      "First temp videos are generated with {} succeeded and {} failed.\n",
      int(finished_count), int(failed_count));
  if (failed_count > 0) {
    return false;
  }

  const int first_temp_count = finished_count;

  std::vector<std::string> second_filenames;
  second_filenames.resize(common.archive_num);
  // make second temp videos
#pragma omp parallel for schedule(dynamic) default(none),                   \
    shared(common, ct, rt, vt, finished_count, failed_count, dry_run, lock, \
               second_filenames)
  for (int aidx = 0; aidx < common.archive_num; aidx++) {
    if (!this->make_second_temp_video(aidx, dry_run)) {
      std::lock_guard<std::mutex> lkgd{lock};
      fmt::print("Failed to make second temp video for archive {}.\n", aidx);
      failed_count++;
      continue;
    }
    finished_count++;
    second_filenames[aidx] = this->video_second_temp_filename(aidx);
  }

  fmt::print(
      "Second temp videos are generated with {} succeeded and {} failed.\n",
      int(finished_count) - first_temp_count, int(failed_count));
  if (failed_count > 0) {
    return false;
  }

  const std::string concate_source_file =
      fmt::format("{}concate_source.txt", vt.temp_config.video_prefix);
  if (!this->make_second_temp_list_txt(concate_source_file, second_filenames,
                                       dry_run)) {
    fmt::print("Failed to generate {}.\n", concate_source_file);
    return false;
  }

  if (!this->make_product_video(concate_source_file, dry_run)) {
    fmt::print("Failed to generate product video.\n");
    return false;
  }

  return true;
}

int fractal_utils::run_command(std::string_view command,
                               bool dry_run) noexcept {
  std::mutex lock;
  if (dry_run) {
    std::lock_guard<std::mutex> lkgd{lock};
    fmt::print("{}\n", command);
    return 0;
  }
  return system(command.data());
}

bool video_executor_base::make_temp_video(int aidx,
                                          bool dry_run) const noexcept {
  const auto &common = *this->m_task.common;
  const auto &ct = *this->m_task.compute;
  const auto &rt = *this->m_task.render;
  const auto &vt = *this->m_task.video;

  const int fps = rt.image_per_frame;

  std::string out_filename = this->video_temp_filename(aidx, false);

  if (can_be_regular_file(out_filename)) {
    return true;
  }

  std::string image_filename_expr;
  this->image_filename_4ffmpeg(aidx, false, image_filename_expr);

  std::string command;
  command = fmt::format(
      "{} -loglevel quiet -r {} -f image2 -start_number 0 -i {} -frames:v "
      "{} -vf "
      "\"scale={}\" {} -y {}",
      vt.ffmpeg_exe, fps, image_filename_expr, fps,
      common.size_expression_4ffmpeg(), vt.temp_config.encode_expr_4ffmpeg(),
      out_filename);

  return run_command(command, dry_run);
}

bool video_executor_base::make_temp_extra_video(int aidx,
                                                bool dry_run) const noexcept {
  const auto &common = *this->m_task.common;
  const auto &ct = *this->m_task.compute;
  const auto &rt = *this->m_task.render;
  const auto &vt = *this->m_task.video;

  const int fps = rt.image_per_frame;
  // extra videos of the last archive will not be used.
  if (aidx == common.archive_num - 1) {
    return true;
  }

  if (rt.extra_image_num <= 0) {
    return true;
  }

  const std::string out_filename = this->video_temp_filename(aidx, true);

  if (can_be_regular_file(out_filename)) {
    return true;
  }

  std::string image_filename_expr;
  this->image_filename_4ffmpeg(aidx, true, image_filename_expr);

  const std::string command = fmt::format(
      "{} -loglevel quiet -r {} -f image2 -start_number {} -i {} -frames:v "
      "{} -vf "
      "\"scale={}\" {} -y {}",
      vt.ffmpeg_exe, fps, fps, image_filename_expr, rt.extra_image_num,
      common.size_expression_4ffmpeg(), vt.temp_config.encode_expr_4ffmpeg(),
      out_filename);

  return run_command(command, dry_run);
}

bool video_executor_base::make_second_temp_video(int aidx,
                                                 bool dry_run) const noexcept {
  const std::string out_filename = this->video_second_temp_filename(aidx);

  if (can_be_regular_file(out_filename)) {
    return true;
  }

  std::string temp_filename;
  this->video_temp_filename(aidx, false, temp_filename);
  std::mutex lock;
  if (aidx <= 0) {
    if (dry_run) {
      std::lock_guard<std::mutex> lkgd{lock};
      fmt::print("Make a symlink or copy file : {} -> {}\n", out_filename,
                 temp_filename);
      return true;
    } else {
      std::error_code ec;
      if (this->m_task.video->prefer_symlink) {
        stdfs::create_symlink(temp_filename, out_filename, ec);
      } else {
        stdfs::copy_file(temp_filename, out_filename, ec);
      }

      if (ec.value() != 0) {
        std::lock_guard<std::mutex> lkgd{lock};
        fmt::print(
            "Failed to copy/create-symlink from {} to {}, error_code = {}, "
            "message = {}.\n",
            temp_filename, out_filename, ec.value(), ec.message());
        return false;
      }
      return true;
    }
  }

  std::string temp_extra_filename;
  this->video_temp_filename(aidx - 1, true, temp_extra_filename);

  const auto &common = *this->m_task.common;
  const auto &ct = *this->m_task.compute;
  const auto &rt = *this->m_task.render;
  const auto &vt = *this->m_task.video;

  const int fps = rt.image_per_frame;

  const std::string rgb_expr =
      fmt::format("'gte({0},N)*({0}.0-N)/{1}.0*255'", rt.extra_image_num,
                  rt.extra_image_num + 1);
  const std::string geq_expr = fmt::format("geq=r={0}:g={0}:b={0}", rgb_expr);

  const std::string i0_expr = fmt::format("-i {}", temp_filename);
  const std::string i1_expr = fmt::format("-i {}", temp_extra_filename);
  const std::string i2_expr =
      fmt::format("-f lavfi -i nullsrc=size={}:r={}:duration=1",
                  common.size_expression_4ffmpeg(), fps);

  const std::string filter_expr = fmt::format(
      "[2]{}[alpha];[1][alpha]alphamerge[extra_a];[0][extra_a]overlay[out]",
      geq_expr);

  const std::string command = fmt::format(
      "{} -loglevel quiet {} {} {} -filter_complex \"{}\" {} -map \"[out]\" "
      "-y {}",
      vt.ffmpeg_exe, i0_expr, i1_expr, i2_expr, filter_expr,
      vt.temp_config.encode_expr_4ffmpeg(), out_filename);

  return run_command(command, dry_run);
}

bool video_executor_base::make_second_temp_list_txt(
    std::string_view txt_filename, std::span<const std::string> concate_sources,
    bool dry_run) const noexcept {
  assert(concate_sources.size() == this->m_task.common->archive_num);

  if (dry_run) {
    fmt::print("Write concate sources to {}\n", txt_filename);
    return true;
  }

  std::ofstream ofs{txt_filename.data()};
  if (!ofs) {
    return false;
  }

  for (const auto &filename : concate_sources) {
    fmt::format_to(std::ostream_iterator<char>{ofs, nullptr}, "file \'{}\'\n",
                   stdfs::canonical(filename).string());
  }

  ofs.close();
  return true;
}
bool video_executor_base::make_product_video(std::string_view txt_filename,
                                             bool dry_run) const noexcept {
  const std::string product_filename = this->product_filename();

  const auto &common = *this->m_task.common;
  const auto &ct = *this->m_task.compute;
  const auto &rt = *this->m_task.render;
  const auto &vt = *this->m_task.video;

  const std::string command = fmt::format(
      "{} -f concat -safe 0 -i {} {} -y {}", vt.ffmpeg_exe, txt_filename,
      vt.product_config.encode_expr_4ffmpeg(), product_filename);

  return run_command(command, dry_run);
}