#include "config/Config.hpp"

#include <nlohmann/json.hpp>

#include <cstdlib>
#include <fstream>

namespace stacktrace {

using nlohmann::json;

std::string Config::ExpandHome(const std::string& path) {
  if (path.rfind("~/", 0) != 0) return path;
  const char* home = std::getenv("HOME");
  if (!home) return path;
  return std::string(home) + path.substr(1);
}

std::string Config::DefaultPath() {
  return ExpandHome("~/.config/stacktrace/config.json");
}

Config Config::Load(const std::string& path) {
  Config cfg;
  std::ifstream f(path);
  if (!f) return cfg;  // defaults

  json j;
  try {
    f >> j;
  } catch (const json::exception&) {
    return cfg;  // malformed → defaults
  }

  cfg.layout = j.value("layout", cfg.layout);
  cfg.refresh_rate_ms = j.value("refresh_rate_ms", cfg.refresh_rate_ms);
  cfg.notes_file = j.value("notes_file", cfg.notes_file);
  cfg.calendar_source = j.value("calendar_source", cfg.calendar_source);
  if (j.contains("tickers")) cfg.tickers = j["tickers"].get<std::vector<std::string>>();
  if (j.contains("rss_feeds")) cfg.rss_feeds = j["rss_feeds"].get<std::vector<std::string>>();
  return cfg;
}

}  // namespace stacktrace
