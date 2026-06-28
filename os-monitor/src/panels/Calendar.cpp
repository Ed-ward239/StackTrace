#include "panels/Calendar.hpp"

#include <cstdio>
#include <ctime>

namespace stacktrace {

using namespace ftxui;

CalendarPanel::CalendarPanel(std::string source) : source_(std::move(source)) {}

void CalendarPanel::Update() {
  // TODO: parse .ics from source_ (or CalDAV via libcurl) into events_.
  // The local iCal parser lands in Phase 4 of the v2 roadmap.
}

Element CalendarPanel::Render() {
  std::time_t now = std::time(nullptr);
  std::tm tm{};
#if defined(_WIN32)
  localtime_s(&tm, &now);
#else
  localtime_r(&now, &tm);
#endif
  int today = tm.tm_mday;
  int month = tm.tm_mon;
  int year = tm.tm_year + 1900;

  // First weekday of the month (0 = Sunday).
  std::tm first = tm;
  first.tm_mday = 1;
  std::mktime(&first);
  int start_wday = first.tm_wday;

  static const int kDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int days = kDays[month];
  if (month == 1 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)) days = 29;

  char title[32];
  std::strftime(title, sizeof(title), "%B %Y", &tm);

  Elements rows;
  rows.push_back(text(title) | bold | hcenter);
  rows.push_back(text("Su Mo Tu We Th Fr Sa") | dim);

  std::string line;
  for (int i = 0; i < start_wday; ++i) line += "   ";
  for (int d = 1; d <= days; ++d) {
    char cell[4];
    std::snprintf(cell, sizeof(cell), "%2d ", d);
    line += cell;
    if ((start_wday + d) % 7 == 0) {
      rows.push_back(text(line));
      line.clear();
    }
  }
  if (!line.empty()) rows.push_back(text(line));

  rows.push_back(separator());
  rows.push_back(text("Upcoming") | bold);
  if (events_.empty()) {
    rows.push_back(text("(no events — see Phase 4)") | dim);
  } else {
    for (const auto& e : events_)
      rows.push_back(text(e.date + "  " + e.summary));
  }
  (void)today;  // highlighting today is a Phase-4 polish item
  return vbox(std::move(rows));
}

}  // namespace stacktrace
