#pragma once

#include "panels/Panel.hpp"

#include <chrono>
#include <string>
#include <vector>

namespace stacktrace::net {
class HttpClient;
}

namespace stacktrace {

// Aggregates headlines from the configured RSS/Atom feeds.
class NewsReaderPanel : public Panel {
 public:
  NewsReaderPanel(std::vector<std::string> feeds, net::HttpClient* http);

  ftxui::Element Render() override;
  void Update() override;  // refetches at most once every 5 minutes
  std::string Title() const override { return "News"; }

 private:
  void Fetch();

  std::vector<std::string> feeds_;
  std::vector<std::string> headlines_;
  net::HttpClient* http_;
  std::chrono::steady_clock::time_point last_fetch_{};
};

}  // namespace stacktrace
