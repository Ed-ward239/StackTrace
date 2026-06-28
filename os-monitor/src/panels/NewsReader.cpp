#include "panels/NewsReader.hpp"

#include "net/HttpClient.hpp"
#include "net/RssParser.hpp"

namespace stacktrace {

using namespace ftxui;

NewsReaderPanel::NewsReaderPanel(std::vector<std::string> feeds,
                                 net::HttpClient* http)
    : feeds_(std::move(feeds)), http_(http) {}

void NewsReaderPanel::Update() {
  auto now = std::chrono::steady_clock::now();
  if (last_fetch_.time_since_epoch().count() != 0 &&
      now - last_fetch_ < std::chrono::minutes(5))
    return;
  last_fetch_ = now;
  Fetch();
}

void NewsReaderPanel::Fetch() {
  if (!http_) return;
  std::vector<std::string> fresh;
  for (const auto& url : feeds_) {
    auto body = http_->Get(url);
    if (!body) continue;
    for (auto& item : net::ParseFeed(*body)) {
      if (item.title.empty()) continue;
      fresh.push_back(item.title);
      if (fresh.size() >= 20) break;
    }
  }
  if (!fresh.empty()) headlines_ = std::move(fresh);
}

Element NewsReaderPanel::Render() {
  Elements rows;
  if (headlines_.empty()) {
    rows.push_back(text("fetching feeds…") | dim);
  } else {
    for (const auto& h : headlines_)
      rows.push_back(hbox({text("• ") | color(Color::Yellow), text(h)}));
  }
  return vbox(std::move(rows)) | vscroll_indicator | yframe;
}

}  // namespace stacktrace
