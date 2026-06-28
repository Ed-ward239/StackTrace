#pragma once

#include "panels/Panel.hpp"

#include <string>
#include <vector>

namespace stacktrace {

// Displays the contents of the notes file. In-panel editing (an FTXUI Input
// component bound to the file) is a roadmap item.
class NotesPanel : public Panel {
 public:
  explicit NotesPanel(std::string path);

  ftxui::Element Render() override;
  void Update() override;
  std::string Title() const override { return "Notes"; }

 private:
  std::string path_;
  std::vector<std::string> lines_;
};

}  // namespace stacktrace
