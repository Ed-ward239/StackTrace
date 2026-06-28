#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Cross-platform system-metrics abstraction.
//
// Exactly ONE translation unit implements these functions — chosen by CMake
// at configure time (linux.cpp, macos.cpp, or raspberrypi.cpp). Panel code
// includes only this header and never learns which OS it is running on.
namespace stacktrace::platform {

struct CpuSample {
  double usage_percent = 0.0;        // aggregate 0..100
  std::vector<double> per_core;      // per-core 0..100
};

struct MemInfo {
  uint64_t total_kb = 0;
  uint64_t used_kb = 0;
};

struct NetInfo {
  uint64_t rx_bytes = 0;             // cumulative since boot
  uint64_t tx_bytes = 0;
};

struct DiskInfo {
  uint64_t total_bytes = 0;
  uint64_t free_bytes = 0;
};

struct ProcInfo {
  int pid = 0;
  std::string name;
  double cpu_percent = 0.0;
  double mem_mb = 0.0;
};

// Human-readable backend name, e.g. "Linux", "macOS", "Raspberry Pi".
std::string PlatformName();

// CPU utilisation. The aggregate field is computed from two samples taken
// one tick apart, so the first call after start may read 0.
CpuSample ReadCpu();

MemInfo ReadMemory();
NetInfo ReadNetwork();
DiskInfo ReadDisk(const std::string& mount_point = "/");

// Top processes by CPU, descending, capped at `limit`.
std::vector<ProcInfo> ReadProcesses(std::size_t limit = 10);

// CPU package temperature in Celsius; returns NaN when unavailable.
double ReadCpuTemperature();

// Raspberry Pi only: GPIO pin levels (empty on other platforms).
struct GpioPin {
  int number = 0;
  bool high = false;
};
std::vector<GpioPin> ReadGpio();

}  // namespace stacktrace::platform
