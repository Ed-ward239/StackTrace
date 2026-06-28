#pragma once

#include <ctime>
#include <string>
#include <vector>

namespace stacktrace::calendar {

struct ICalEvent {
  std::time_t start = 0;  // epoch seconds
  std::string summary;
  bool all_day = false;
};

// Parses an iCalendar (RFC 5545) document into its VEVENTs, reading SUMMARY and
// DTSTART. Handles line folding, DATE and DATE-TIME values, and the trailing
// 'Z' UTC marker. Returns events in document order; malformed input yields {}.
std::vector<ICalEvent> ParseICal(const std::string& text);

}  // namespace stacktrace::calendar
