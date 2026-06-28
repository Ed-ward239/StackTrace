#pragma once

#include "calendar/ICalParser.hpp"
#include "net/HttpClient.hpp"
#include "panels/Panel.hpp"

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace stacktrace {

// Month grid with the current day highlighted, plus an upcoming-events list.
// Events are loaded from an .ics file, a directory of .ics files, or a CalDAV
// URL (fetched via libcurl) on a background worker thread.
class CalendarPanel : public Panel {
 public:
  explicit CalendarPanel(std::string source);
  ~CalendarPanel() override;

  ftxui::Element Render() override;
  void Update() override {}  // no-op: the worker thread refreshes
  std::string Title() const override { return "Calendar"; }

 private:
  void Worker();
  std::string LoadSource();  // returns concatenated iCal text (or "")

  std::string source_;
  std::vector<calendar::ICalEvent> events_;  // upcoming, sorted; guarded by mutex_
  std::mutex mutex_;
  net::HttpClient http_;
  std::atomic<bool> stop_{false};
  std::thread worker_;
};

}  // namespace stacktrace
