#pragma once

#include "panels/Panel.hpp"

#include <string>
#include <vector>

namespace stacktrace {

// Month grid with the current day highlighted, plus an upcoming-events list.
class CalendarPanel : public Panel {
 public:
  explicit CalendarPanel(std::string source);

  ftxui::Element Render() override;
  void Update() override;
  std::string Title() const override { return "Calendar"; }

 private:
  struct Event {
    std::string date;
    std::string summary;
  };

  std::string source_;          // .ics file or .calcurse/ dir, or CalDAV URL
  std::vector<Event> events_;
};

}  // namespace stacktrace
