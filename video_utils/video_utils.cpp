#include "video_utils.h"
#include <fmt/format.h>
#include <filesystem>
#include <omp.h>

namespace stdfs = std::filesystem;
using namespace fractal_utils;

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
    std::string_view filename, std::any *return_archive) const noexcept {
  std::string err;
  auto ar = this->load_archive(filename, err);
  if (!ar.has_value() || !err.empty()) {
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
    fmt::format_to(std::back_inserter(ret), "{}image{}-{}{}.{}",
                   rt->image_prefix, archive_index, image_idx, rt->image_suffix,
                   rt->image_extension);
  } else {
    fmt::format_to(std::back_inserter(ret), "{}image{}-e{}{}.{}",
                   rt->image_prefix, archive_index, image_idx, rt->image_suffix,
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
    fmt::format_to(std::back_inserter(ret), "{}temp{}{}.{}",
                   vt->temp_config.video_prefix, archive_index,
                   vt->temp_config.video_suffix, vt->temp_config.extension);
  } else {
    fmt::format_to(std::back_inserter(ret), "{}temp-extra{}{}.{}",
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

bool video_executor_base::load_task() noexcept {
  std::string err;
  auto temp = this->load_task(err);
  if (!temp.has_value() || err.empty()) {
    return false;
  }
  this->m_task = std::move(temp.value());
  return true;
}

bool video_executor_base::run_compute() const noexcept {
  const auto &common = *this->m_task.common;
  const auto &ct = *this->m_task.compute;

  omp_set_num_threads(ct.threads);

  // 1 -> finished
  std::vector<uint8_t> task_lut;
  task_lut.resize(common.archive_num);
  std::fill(task_lut.begin(), task_lut.end(), false);

  std::string filename;
  filename.reserve(4096);
  std::any archive;
  for (int aidx = 0; aidx < common.archive_num; aidx++) {
    this->archive_filename(aidx, filename);

    if (!can_be_regular_file(filename)) {
      continue;
    }

    if (this->check_archive(filename, nullptr)) {
      fmt::print(
          "{} exists but is found to be corrupted, it will be generated "
          "again.\n",
          filename);
      continue;
    }
    task_lut[aidx] = true;

    if (!create_required_dirs(filename)) {
      fmt::print(
          "Warning: Failed to create required directories for {}, the "
          "computation may fail.\n",
          filename);
    }
  }

  int finished_tasks{0};
  for (auto status : task_lut) {
    finished_tasks += bool(status);
  }
  const int already_finished_tasks = finished_tasks;

  std::unique_ptr<wind_base> current_wind{ct.start_window->create_another()};

  for (int aidx = 0; aidx < common.archive_num; aidx++) {
    if (task_lut[aidx]) {
      continue;
    }
    this->archive_filename(aidx, filename);
    fmt::print(
        "[{} / {} : {}%] : computing {}", finished_tasks, common.archive_num,
        float(finished_tasks * 100) / float(common.archive_num), filename);

    ct.start_window->copy_to(current_wind.get());
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