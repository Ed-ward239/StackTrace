#include "panels/StockTicker.hpp"

#include <nlohmann/json.hpp>

#include <chrono>
#include <iomanip>
#include <sstream>

namespace stacktrace {

using namespace ftxui;

StockTickerPanel::StockTickerPanel(std::vector<std::string> symbols)
    : symbols_(std::move(symbols)) {
  for (const auto& s : symbols_) quotes_.push_back({s, 0.0, 0.0, false});
  worker_ = std::thread([this] { Worker(); });
}

StockTickerPanel::~StockTickerPanel() {
  stop_ = true;
  if (worker_.joinable()) worker_.join();
}

void StockTickerPanel::Worker() {
  while (!stop_) {
    std::vector<Quote> fresh;
    fresh.reserve(symbols_.size());
    for (const auto& s : symbols_) {
      if (stop_) return;
      fresh.push_back(FetchOne(s));
    }
    {
      std::lock_guard<std::mutex> lk(mutex_);
      quotes_ = std::move(fresh);
    }
    // Sleep ~60s, but wake promptly on shutdown.
    for (int i = 0; i < 60 && !stop_; ++i)
      std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

StockTickerPanel::Quote StockTickerPanel::FetchOne(const std::string& symbol) {
  Quote q{symbol, 0.0, 0.0, false};
  auto body = http_.Get(
      "https://query1.finance.yahoo.com/v8/finance/chart/" + symbol);
  if (!body) return q;
  try {
    auto j = nlohmann::json::parse(*body);
    const auto& meta = j["chart"]["result"][0]["meta"];
    double price = meta.value("regularMarketPrice", 0.0);
    double prev = meta.value("chartPreviousClose", price);
    q.price = price;
    q.change_pct = prev != 0.0 ? 100.0 * (price - prev) / prev : 0.0;
    q.live = true;
  } catch (const nlohmann::json::exception&) {
    // leave as not-live on a parse error
  }
  return q;
}

Element StockTickerPanel::Render() {
  std::lock_guard<std::mutex> lk(mutex_);
  Elements rows;
  for (const auto& q : quotes_) {
    Color c = q.change_pct >= 0 ? Color::Green : Color::Red;
    std::ostringstream price, chg;
    price << std::fixed << std::setprecision(2) << q.price;
    chg << std::showpos << std::fixed << std::setprecision(2) << q.change_pct
        << "%";
    rows.push_back(hbox({
        text(q.symbol) | bold | size(WIDTH, EQUAL, 9),
        text(q.live ? price.str() : "—") | size(WIDTH, EQUAL, 10),
        text(q.live ? chg.str() : "  …") | color(c) | flex | align_right,
    }));
  }
  if (rows.empty()) rows.push_back(text("no symbols configured") | dim);
  return vbox(std::move(rows));
}

}  // namespace stacktrace
