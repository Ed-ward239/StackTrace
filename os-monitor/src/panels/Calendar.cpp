#include "panels/Calendar.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace stacktrace {

using namespace ftxui;
namespace fs = std::filesystem;

CalendarPanel::CalendarPanel(std::string source) : source_(std::move(source)) {
  worker_ = std::thread([this] { Worker(); });
}

CalendarPanel::~CalendarPanel() {
  stop_ = true;
  if (worker_.joinable()) worker_.join();
}

std::string CalendarPanel::LoadSource() {
  // Remote CalDAV / hosted .ics.
  if (source_.rfind("http", 0) == 0) {
    auto body = http_.Get(source_);
    return body.value_or("");
  }

  std::error_code ec;
  if (fs::is_directory(source_, ec)) {
    std::string all;
    for (const auto& de : fs::directory_iterator(source_, ec)) {
      if (de.path().extension() != ".ics") continue;
      std::ifstream f(de.path());
      std::stringstream ss;
      ss << f.rdbuf();
      all += ss.str();
      all += "\n";
    }
    return all;
  }

  std::ifstream f(source_);
  if (!f) return "";
  std::stringstream ss;
  ss << f.rdbuf();
  return ss.str();
}

void CalendarPanel::Worker() {
  while (!stop_) {
    auto parsed = calendar::ParseICal(LoadSource());

    // Keep events from the start of today onward, soonest first.
    std::time_t now = std::time(nullptr);
    std::tm midnight{};
#if defined(_WIN32)
    localtime_s(&midnight, &now);
#else
    localtime_r(&now, &midnight);
#endif
    midnight.tm_hour = midnight.tm_min = midnight.tm_sec = 0;
    std::time_t day_start = std::mktime(&midnight);

    std::vector<calendar::ICalEvent> upcoming;
    for (auto& e : parsed)
      if (e.start >= day_start) upcoming.push_back(std::move(e));
    std::sort(upcoming.begin(), upcoming.end(),
              [](const auto& a, const auto& b) { return a.start < b.start; });

    {
      std::lock_guard<std::mutex> lk(mutex_);
      events_ = std::move(upcoming);
    }

    for (int i = 0; i < 300 && !stop_; ++i)
      std::this_thread::sleep_for(std::chrono::seconds(1));
  }
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

  std::tm first = tm;
  first.tm_mday = 1;
  std::mktime(&first);
  int start_wday = first.tm_wday;

  static const int kDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int days = kDays[month];
  if (month == 1 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0))
    days = 29;

  char title[32];
  std::strftime(title, sizeof(title), "%B %Y", &tm);

  Elements rows;
  rows.push_back(text(title) | bold | hcenter);
  rows.push_back(text("Su Mo Tu We Th Fr Sa") | dim);

  // Build the month grid as one hbox of day cells per week so today's cell can
  // be highlighted individually.
  Elements week;
  for (int i = 0; i < start_wday; ++i) week.push_back(text("   "));
  for (int d = 1; d <= days; ++d) {
    char cell[4];
    std::snprintf(cell, sizeof(cell), "%2d ", d);
    Element e = text(cell);
    if (d == today) e = e | inverted;
    week.push_back(e);
    if ((start_wday + d) % 7 == 0) {
      rows.push_back(hbox(week));
      week.clear();
    }
  }
  if (!week.empty()) rows.push_back(hbox(week));

  rows.push_back(separator());
  rows.push_back(text("Upcoming") | bold);

  std::lock_guard<std::mutex> lk(mutex_);
  if (events_.empty()) {
    rows.push_back(text("(no upcoming events)") | dim);
  } else {
    int shown = 0;
    for (const auto& e : events_) {
      if (shown++ >= 6) break;
      std::tm et{};
#if defined(_WIN32)
      localtime_s(&et, &e.start);
#else
      localtime_r(&e.start, &et);
#endif
      char when[32];
      std::strftime(when, sizeof(when), e.all_day ? "%a %d %b" : "%a %d %b %H:%M",
                    &et);
      rows.push_back(hbox({
          text(std::string(when) + "  ") | color(Color::Cyan),
          text(e.summary) | flex,
      }));
    }
  }
  return vbox(std::move(rows));
}

}  // namespace stacktrace
