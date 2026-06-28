#pragma once

#include "panels/Panel.hpp"

#include <string>

namespace stacktrace {

// Large block-digit clock plus the current date, driven by std::chrono.
class ClockPanel : public Panel {
 public:
  ftxui::Element Render() override;
  void Update() override;
  std::string Title() const override { return "Clock"; }

 private:
  std::string time_;  // "HH:MM:SS"
  std::string date_;  // "Sat 28 Jun 2026"
};

}  // namespace stacktrace
