#pragma once

#include "panels/Panel.hpp"
#include "platform/Metrics.hpp"

#include <chrono>

namespace stacktrace {

// Replaces the shell build's fastfetch + btop panes: live CPU, RAM, disk, network,
// temperature and a top-process list, read directly from the OS.
class SystemMetricsPanel : public Panel {
 public:
  ftxui::Element Render() override;
  void Update() override;
  std::string Title() const override { return "System"; }

 private:
  platform::CpuSample cpu_;
  platform::MemInfo mem_;
  platform::DiskInfo disk_;
  platform::NetInfo net_;
  platform::NetInfo prev_net_;
  std::vector<platform::ProcInfo> procs_;
  double temp_c_ = 0.0;
  double net_rx_kbps_ = 0.0;
  double net_tx_kbps_ = 0.0;
  std::chrono::steady_clock::time_point last_net_{};
};

}  // namespace stacktrace
