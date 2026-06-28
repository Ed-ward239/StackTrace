#include "panels/NewsReader.hpp"

#include <chrono>

#include "net/RssParser.hpp"

namespace stacktrace {

using namespace ftxui;

NewsReaderPanel::NewsReaderPanel(std::vector<std::string> feeds)
    : feeds_(std::move(feeds)) {
  worker_ = std::thread([this] { Worker(); });
}

NewsReaderPanel::~NewsReaderPanel() {
  stop_ = true;
  if (worker_.joinable()) worker_.join();
}

void NewsReaderPanel::Worker() {
  while (!stop_) {
    std::vector<std::string> fresh;
    for (const auto& url : feeds_) {
      if (stop_) return;
      auto body = http_.Get(url);
      if (!body) continue;
      for (auto& item : net::ParseFeed(*body)) {
        if (item.title.empty()) continue;
        fresh.push_back(item.title);
        if (fresh.size() >= 20) break;
      }
    }
    if (!fresh.empty()) {
      std::lock_guard<std::mutex> lk(mutex_);
      headlines_ = std::move(fresh);
    }
    // Sleep ~5 min, waking promptly on shutdown.
    for (int i = 0; i < 300 && !stop_; ++i)
      std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

Element NewsReaderPanel::Render() {
  std::lock_guard<std::mutex> lk(mutex_);
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
