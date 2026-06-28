#pragma once

#include <string>
#include <vector>

namespace stacktrace {

// User configuration, loaded from ~/.config/stacktrace/config.json.
// Missing keys fall back to the defaults below.
struct Config {
  std::string layout = "default";
  int refresh_rate_ms = 1000;
  std::vector<std::string> tickers = {"AAPL", "NVDA", "TSLA", "BTC-USD"};
  std::vector<std::string> rss_feeds = {
      "https://feeds.bbci.co.uk/news/rss.xml",
      "https://www.theverge.com/rss/index.xml"};
  std::string notes_file = "~/notes.txt";
  std::string calendar_source = "~/.calcurse/";

  // Loads config from `path` (defaults to the standard location).
  // Always returns a usable Config; on error it returns the defaults.
  static Config Load(const std::string& path = DefaultPath());

  // Expands a leading "~/" to the user's home directory.
  static std::string ExpandHome(const std::string& path);

  static std::string DefaultPath();
};

}  // namespace stacktrace
