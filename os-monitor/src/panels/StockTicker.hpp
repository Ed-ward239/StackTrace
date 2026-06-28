#pragma once

#include "panels/Panel.hpp"

#include <chrono>
#include <string>
#include <vector>

namespace stacktrace::net {
class HttpClient;
}

namespace stacktrace {

class StockTickerPanel : public Panel {
 public:
  StockTickerPanel(std::vector<std::string> symbols, net::HttpClient* http);

  ftxui::Element Render() override;
  void Update() override;  // polls Yahoo Finance at most once per minute
  std::string Title() const override { return "Markets"; }

 private:
  struct Quote {
    std::string symbol;
    double price = 0.0;
    double change_pct = 0.0;
    bool live = false;
  };

  void Fetch();

  std::vector<std::string> symbols_;
  std::vector<Quote> quotes_;
  net::HttpClient* http_;
  std::chrono::steady_clock::time_point last_fetch_{};
};

}  // namespace stacktrace
