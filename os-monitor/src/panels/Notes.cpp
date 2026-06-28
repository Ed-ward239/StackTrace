#include "panels/Notes.hpp"

#include <fstream>

namespace stacktrace {

using namespace ftxui;

NotesPanel::NotesPanel(std::string path) : path_(std::move(path)) {}

void NotesPanel::Update() {
  std::ifstream f(path_);
  if (!f) return;
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(f, line)) lines.push_back(line);
  lines_ = std::move(lines);
}

Element NotesPanel::Render() {
  Elements rows;
  if (lines_.empty()) {
    rows.push_back(text("notes file is empty") | dim);
    rows.push_back(text(path_) | dim);
  } else {
    for (const auto& l : lines_) rows.push_back(text(l));
  }
  return vbox(std::move(rows)) | vscroll_indicator | yframe;
}

}  // namespace stacktrace
