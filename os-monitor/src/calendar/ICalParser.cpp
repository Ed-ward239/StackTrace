#include "calendar/ICalParser.hpp"

#include <cstdlib>
#include <sstream>

namespace stacktrace::calendar {
namespace {

// Days since the Unix epoch for a proleptic-Gregorian date (Howard Hinnant's
// algorithm). Lets us convert UTC iCal timestamps without the non-standard
// timegm().
long long DaysFromCivil(int y, unsigned m, unsigned d) {
  y -= m <= 2;
  const long long era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = static_cast<unsigned>(y - era * 400);
  const unsigned doy = (153u * (m + (m > 2 ? -3u : 9u)) + 2u) / 5u + d - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + static_cast<long long>(doe) - 719468;
}

std::time_t TimegmUtc(const std::tm& tm) {
  long long days = DaysFromCivil(tm.tm_year + 1900,
                                 static_cast<unsigned>(tm.tm_mon + 1),
                                 static_cast<unsigned>(tm.tm_mday));
  return static_cast<std::time_t>(days * 86400 + tm.tm_hour * 3600 +
                                  tm.tm_min * 60 + tm.tm_sec);
}

// RFC 5545 line folding: a CRLF followed by a space or tab continues the
// previous logical line. Unfold into a flat list of logical lines.
std::vector<std::string> Unfold(const std::string& text) {
  std::vector<std::string> lines;
  std::istringstream ss(text);
  std::string raw;
  while (std::getline(ss, raw)) {
    if (!raw.empty() && raw.back() == '\r') raw.pop_back();
    if (!lines.empty() && !raw.empty() && (raw[0] == ' ' || raw[0] == '\t'))
      lines.back() += raw.substr(1);
    else
      lines.push_back(raw);
  }
  return lines;
}

// Convert "20260628" or "20260628T140000Z" into epoch seconds.
std::time_t ParseDateTime(const std::string& value, bool& all_day) {
  if (value.size() < 8) return 0;
  auto num = [&](int pos, int len) {
    return std::atoi(value.substr(pos, len).c_str());
  };
  std::tm tm{};
  tm.tm_year = num(0, 4) - 1900;
  tm.tm_mon = num(4, 2) - 1;
  tm.tm_mday = num(6, 2);

  bool utc = false;
  if (value.size() >= 15 && value[8] == 'T') {
    all_day = false;
    tm.tm_hour = num(9, 2);
    tm.tm_min = num(11, 2);
    tm.tm_sec = num(13, 2);
    utc = value.back() == 'Z';
  } else {
    all_day = true;  // DATE value, no time component
  }
  return utc ? TimegmUtc(tm) : std::mktime(&tm);
}

}  // namespace

std::vector<ICalEvent> ParseICal(const std::string& text) {
  std::vector<ICalEvent> events;
  bool in_event = false;
  ICalEvent cur;

  for (const auto& line : Unfold(text)) {
    if (line == "BEGIN:VEVENT") {
      in_event = true;
      cur = ICalEvent{};
      continue;
    }
    if (line == "END:VEVENT") {
      if (in_event && cur.start != 0) events.push_back(cur);
      in_event = false;
      continue;
    }
    if (!in_event) continue;

    // Split "NAME;params:value" into property name and value.
    auto colon = line.find(':');
    if (colon == std::string::npos) continue;
    std::string name = line.substr(0, colon);
    std::string value = line.substr(colon + 1);
    auto semi = name.find(';');
    if (semi != std::string::npos) name = name.substr(0, semi);

    if (name == "SUMMARY")
      cur.summary = value;
    else if (name == "DTSTART")
      cur.start = ParseDateTime(value, cur.all_day);
  }
  return events;
}

}  // namespace stacktrace::calendar
