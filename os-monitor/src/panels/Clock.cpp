#include "panels/Clock.hpp"

#include <array>
#include <chrono>
#include <ctime>

namespace stacktrace {

using namespace ftxui;

namespace {

// 5-row block font for digits 0-9 and ':'.
const std::array<std::array<const char*, 5>, 11> kFont = {{
    {{" ███ ", "█   █", "█   █", "█   █", " ███ "}},  // 0
    {{"  █  ", " ██  ", "  █  ", "  █  ", " ███ "}},  // 1
    {{" ███ ", "█   █", "  ██ ", " █   ", "█████"}},  // 2
    {{"████ ", "    █", " ███ ", "    █", "████ "}},  // 3
    {{"█  █ ", "█  █ ", "█████", "   █ ", "   █ "}},  // 4
    {{"█████", "█    ", "████ ", "    █", "████ "}},  // 5
    {{" ███ ", "█    ", "████ ", "█   █", " ███ "}},  // 6
    {{"█████", "    █", "   █ ", "  █  ", " █   "}},  // 7
    {{" ███ ", "█   █", " ███ ", "█   █", " ███ "}},  // 8
    {{" ███ ", "█   █", " ████", "    █", " ███ "}},  // 9
    {{"     ", "  █  ", "     ", "  █  ", "     "}},  // :
}};

int GlyphIndex(char c) { return c == ':' ? 10 : c - '0'; }

}  // namespace

void ClockPanel::Update() {
  std::time_t now = std::time(nullptr);
  std::tm tm{};
#if defined(_WIN32)
  localtime_s(&tm, &now);
#else
  localtime_r(&now, &tm);
#endif
  char buf[32];
  std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
  time_ = buf;
  std::strftime(buf, sizeof(buf), "%a %d %b %Y", &tm);
  date_ = buf;
}

Element ClockPanel::Render() {
  Elements rows;
  for (int r = 0; r < 5; ++r) {
    std::string line;
    for (char c : time_) {
      line += kFont[GlyphIndex(c)][r];
      line += ' ';
    }
    rows.push_back(text(line) | color(Color::CyanLight) | hcenter);
  }
  rows.push_back(text("") );
  rows.push_back(text(date_) | bold | hcenter);
  return vbox(std::move(rows)) | center;
}

}  // namespace stacktrace
