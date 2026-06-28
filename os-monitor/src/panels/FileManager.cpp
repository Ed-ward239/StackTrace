#include "panels/FileManager.hpp"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <system_error>

namespace stacktrace {

using namespace ftxui;
namespace fs = std::filesystem;

FileManagerPanel::FileManagerPanel(std::string start_dir)
    : dir_(std::move(start_dir)) {}

void FileManagerPanel::Update() {
  entries_.clear();
  std::error_code ec;
  for (const auto& de : fs::directory_iterator(dir_, ec)) {
    Entry e;
    e.name = de.path().filename().string();
    e.is_dir = de.is_directory(ec);
    if (!e.is_dir) e.size = de.file_size(ec);
    entries_.push_back(std::move(e));
  }
  std::sort(entries_.begin(), entries_.end(), [](const Entry& a, const Entry& b) {
    if (a.is_dir != b.is_dir) return a.is_dir;  // directories first
    return a.name < b.name;
  });
}

namespace {
std::string HumanSize(uintmax_t bytes) {
  const char* units[] = {"B", "K", "M", "G", "T"};
  double v = double(bytes);
  int u = 0;
  while (v >= 1024.0 && u < 4) {
    v /= 1024.0;
    ++u;
  }
  char buf[16];
  std::snprintf(buf, sizeof(buf), v < 10 && u > 0 ? "%.1f%s" : "%.0f%s", v, units[u]);
  return buf;
}
}  // namespace

Element FileManagerPanel::Render() {
  Elements rows;
  rows.push_back(text(dir_) | dim);
  rows.push_back(separator());
  for (const auto& e : entries_) {
    Element name = e.is_dir ? text("📁 " + e.name) | color(Color::Blue)
                            : text("   " + e.name);
    rows.push_back(hbox({
        name | flex,
        text(e.is_dir ? "" : HumanSize(e.size)) | align_right,
    }));
  }
  if (entries_.empty()) rows.push_back(text("(empty)") | dim);
  return vbox(std::move(rows)) | vscroll_indicator | yframe;
}

}  // namespace stacktrace
