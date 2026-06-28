#pragma once

#include "net/HttpClient.hpp"
#include "panels/Panel.hpp"

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace stacktrace {

// Live stock/crypto quotes from Yahoo Finance. A background worker polls once
// per minute so the render loop never blocks on the network; Render() reads a
// snapshot under a short lock.
class StockTickerPanel : public Panel {
 public:
  explicit StockTickerPanel(std::vector<std::string> symbols);
  ~StockTickerPanel() override;

  ftxui::Element Render() override;
  void Update() override {}  // no-op: the worker thread refreshes
  std::string Title() const override { return "Markets"; }

 private:
  struct Quote {
    std::string symbol;
    double price = 0.0;
    double change_pct = 0.0;
    bool live = false;
  };

  void Worker();
  Quote FetchOne(const std::string& symbol);

  std::vector<std::string> symbols_;
  std::vector<Quote> quotes_;  // guarded by mutex_
  std::mutex mutex_;
  net::HttpClient http_;
  std::atomic<bool> stop_{false};
  std::thread worker_;
};

}  // namespace stacktrace
