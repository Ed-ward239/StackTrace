#pragma once

#include "net/HttpClient.hpp"
#include "panels/Panel.hpp"

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace stacktrace {

// Aggregates headlines from the configured RSS/Atom feeds. A background worker
// refetches every few minutes; Render() reads a snapshot under a short lock.
class NewsReaderPanel : public Panel {
 public:
  explicit NewsReaderPanel(std::vector<std::string> feeds);
  ~NewsReaderPanel() override;

  ftxui::Element Render() override;
  void Update() override {}  // no-op: the worker thread refreshes
  std::string Title() const override { return "News"; }

 private:
  void Worker();

  std::vector<std::string> feeds_;
  std::vector<std::string> headlines_;  // guarded by mutex_
  std::mutex mutex_;
  net::HttpClient http_;
  std::atomic<bool> stop_{false};
  std::thread worker_;
};

}  // namespace stacktrace
