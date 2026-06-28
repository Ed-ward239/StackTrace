#include "panels/SystemMetrics.hpp"

#include <cmath>
#include <sstream>

namespace stacktrace {

using namespace ftxui;

namespace {
std::string Pct(double v) {
  std::ostringstream ss;
  ss.precision(0);
  ss << std::fixed << v << "%";
  return ss.str();
}
}  // namespace

void SystemMetricsPanel::Update() {
  cpu_ = platform::ReadCpu();
  mem_ = platform::ReadMemory();
  disk_ = platform::ReadDisk("/");
  temp_c_ = platform::ReadCpuTemperature();
  procs_ = platform::ReadProcesses(6);

  platform::NetInfo now = platform::ReadNetwork();
  auto t = std::chrono::steady_clock::now();
  if (last_net_.time_since_epoch().count() != 0) {
    double secs = std::chrono::duration<double>(t - last_net_).count();
    if (secs > 0) {
      net_rx_kbps_ = (now.rx_bytes - prev_net_.rx_bytes) / 1024.0 / secs;
      net_tx_kbps_ = (now.tx_bytes - prev_net_.tx_bytes) / 1024.0 / secs;
    }
  }
  prev_net_ = now;
  net_ = now;
  last_net_ = t;
}

Element SystemMetricsPanel::Render() {
  double mem_pct =
      mem_.total_kb ? 100.0 * double(mem_.used_kb) / double(mem_.total_kb) : 0.0;
  double disk_pct =
      disk_.total_bytes
          ? 100.0 * double(disk_.total_bytes - disk_.free_bytes) / double(disk_.total_bytes)
          : 0.0;

  auto metric = [](const std::string& label, double pct, Color c) {
    return hbox({
        text(label) | size(WIDTH, EQUAL, 5),
        gauge(float(pct / 100.0)) | color(c) | flex,
        text(" " + Pct(pct)) | size(WIDTH, EQUAL, 5),
    });
  };

  Elements cores;
  for (std::size_t i = 0; i < cpu_.per_core.size(); ++i)
    cores.push_back(metric("c" + std::to_string(i), cpu_.per_core[i], Color::Cyan));

  Elements proc_rows;
  proc_rows.push_back(hbox({text("PID") | size(WIDTH, EQUAL, 7),
                            text("MEM") | size(WIDTH, EQUAL, 9), text("NAME")}) |
                      dim);
  for (const auto& p : procs_) {
    std::ostringstream mb;
    mb.precision(1);
    mb << std::fixed << p.mem_mb << "M";
    proc_rows.push_back(hbox({
        text(std::to_string(p.pid)) | size(WIDTH, EQUAL, 7),
        text(mb.str()) | size(WIDTH, EQUAL, 9),
        text(p.name) | flex,
    }));
  }

  std::ostringstream net;
  net.precision(0);
  net << std::fixed << "net  ↓" << net_rx_kbps_ << " ↑" << net_tx_kbps_ << " KB/s";

  Element temp = std::isnan(temp_c_)
                     ? text("temp n/a") | dim
                     : text("temp " + Pct(temp_c_).substr(0, Pct(temp_c_).size() - 1) + "°C");

  return vbox({
      metric("cpu", cpu_.usage_percent, Color::Green),
      vbox(std::move(cores)),
      separator(),
      metric("mem", mem_pct, Color::Yellow),
      metric("disk", disk_pct, Color::Magenta),
      hbox({text(net.str()), filler(), temp}),
      separator(),
      vbox(std::move(proc_rows)) | flex,
  });
}

}  // namespace stacktrace
