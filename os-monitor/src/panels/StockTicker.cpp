#include "panels/StockTicker.hpp"

#include <nlohmann/json.hpp>

#include <iomanip>
#include <sstream>

#include "net/HttpClient.hpp"

namespace stacktrace {

using namespace ftxui;

StockTickerPanel::StockTickerPanel(std::vector<std::string> symbols,
                                   net::HttpClient* http)
    : symbols_(std::move(symbols)), http_(http) {
  for (const auto& s : symbols_) quotes_.push_back({s, 0.0, 0.0, false});
}

void StockTickerPanel::Update() {
  auto now = std::chrono::steady_clock::now();
  if (last_fetch_.time_since_epoch().count() != 0 &&
      now - last_fetch_ < std::chrono::seconds(60))
    return;
  last_fetch_ = now;
  Fetch();
}

void StockTickerPanel::Fetch() {
  for (auto& q : quotes_) {
    if (!http_) break;
    auto body = http_->Get(
        "https://query1.finance.yahoo.com/v8/finance/chart/" + q.symbol);
    if (!body) continue;  // keep previous value / placeholder
    try {
      auto j = nlohmann::json::parse(*body);
      const auto& meta = j["chart"]["result"][0]["meta"];
      double price = meta.value("regularMarketPrice", 0.0);
      double prev = meta.value("chartPreviousClose", price);
      q.price = price;
      q.change_pct = prev != 0.0 ? 100.0 * (price - prev) / prev : 0.0;
      q.live = true;
    } catch (const nlohmann::json::exception&) {
      // leave the quote untouched on a parse error
    }
  }
}

Element StockTickerPanel::Render() {
  Elements rows;
  for (const auto& q : quotes_) {
    Color c = q.change_pct >= 0 ? Color::Green : Color::Red;
    std::ostringstream price, chg;
    price << std::fixed << std::setprecision(2) << q.price;
    chg << std::showpos << std::fixed << std::setprecision(2) << q.change_pct << "%";
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
