#pragma once

#include "panels/Panel.hpp"

#include <string>
#include <vector>

namespace stacktrace {

// Read-only directory browser. A future interactive version (key-driven
// navigation, previews) is noted in the roadmap.
class FileManagerPanel : public Panel {
 public:
  explicit FileManagerPanel(std::string start_dir);

  ftxui::Element Render() override;
  void Update() override;
  std::string Title() const override { return "Files"; }

 private:
  struct Entry {
    std::string name;
    bool is_dir = false;
    uintmax_t size = 0;
  };

  std::string dir_;
  std::vector<Entry> entries_;
};

}  // namespace stacktrace
