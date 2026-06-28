#pragma once
// Shared /proc + /sys readers for the Linux family of backends.
// Both linux.cpp and raspberrypi.cpp delegate to these inline helpers so the
// generic logic lives in one place; the Pi backend layers GPIO/thermal on top.
#if defined(__linux__)

#include "platform/Metrics.hpp"

#include <dirent.h>
#include <sys/statvfs.h>
#include <unistd.h>

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

namespace stacktrace::platform::proclinux {

struct CpuTimes {
  uint64_t idle = 0;
  uint64_t total = 0;
};

inline CpuTimes ParseCpuLine(const std::string& line) {
  std::istringstream ss(line);
  std::string cpu;
  ss >> cpu;
  CpuTimes t;
  uint64_t v, idx = 0;
  while (ss >> v) {
    t.total += v;
    if (idx == 3 || idx == 4) t.idle += v;  // idle + iowait
    ++idx;
  }
  return t;
}

inline CpuSample ReadCpu() {
  static CpuTimes prev_total;
  static std::vector<CpuTimes> prev_cores;

  std::ifstream f("/proc/stat");
  std::string line;
  CpuSample sample;
  std::vector<CpuTimes> cores;

  while (std::getline(f, line)) {
    if (line.rfind("cpu", 0) != 0) break;
    CpuTimes t = ParseCpuLine(line);
    if (line[3] == ' ') {
      uint64_t di = t.idle - prev_total.idle, dt = t.total - prev_total.total;
      if (dt > 0) sample.usage_percent = 100.0 * (1.0 - double(di) / double(dt));
      prev_total = t;
    } else {
      cores.push_back(t);
    }
  }

  for (std::size_t i = 0; i < cores.size(); ++i) {
    double pct = 0.0;
    if (i < prev_cores.size()) {
      uint64_t di = cores[i].idle - prev_cores[i].idle;
      uint64_t dt = cores[i].total - prev_cores[i].total;
      if (dt > 0) pct = 100.0 * (1.0 - double(di) / double(dt));
    }
    sample.per_core.push_back(pct);
  }
  prev_cores = cores;
  return sample;
}

inline MemInfo ReadMemory() {
  std::ifstream f("/proc/meminfo");
  std::string key, unit;
  uint64_t value;
  MemInfo m;
  uint64_t avail = 0;
  while (f >> key >> value >> unit) {
    if (key == "MemTotal:") m.total_kb = value;
    else if (key == "MemAvailable:") avail = value;
  }
  m.used_kb = (m.total_kb > avail) ? m.total_kb - avail : 0;
  return m;
}

inline NetInfo ReadNetwork() {
  std::ifstream f("/proc/net/dev");
  std::string line;
  NetInfo n;
  std::getline(f, line);
  std::getline(f, line);
  while (std::getline(f, line)) {
    auto colon = line.find(':');
    if (colon == std::string::npos) continue;
    std::string iface = line.substr(0, colon);
    iface.erase(0, iface.find_first_not_of(" \t"));
    if (iface == "lo") continue;
    std::istringstream ss(line.substr(colon + 1));
    uint64_t rx, tx, skip;
    ss >> rx;
    for (int i = 0; i < 7; ++i) ss >> skip;
    ss >> tx;
    n.rx_bytes += rx;
    n.tx_bytes += tx;
  }
  return n;
}

inline DiskInfo ReadDisk(const std::string& mount_point) {
  DiskInfo d;
  struct statvfs s {};
  if (statvfs(mount_point.c_str(), &s) == 0) {
    d.total_bytes = uint64_t(s.f_blocks) * s.f_frsize;
    d.free_bytes = uint64_t(s.f_bavail) * s.f_frsize;
  }
  return d;
}

inline std::vector<ProcInfo> ReadProcesses(std::size_t limit) {
  std::vector<ProcInfo> procs;
  DIR* dir = opendir("/proc");
  if (!dir) return procs;
  const long page_kb = sysconf(_SC_PAGESIZE) / 1024;
  struct dirent* e;
  while ((e = readdir(dir)) != nullptr) {
    if (e->d_name[0] < '0' || e->d_name[0] > '9') continue;
    std::ifstream st(std::string("/proc/") + e->d_name + "/stat");
    if (!st) continue;
    ProcInfo p;
    std::string comm, tok;
    char ch;
    st >> p.pid;
    st >> std::ws;
    st.get(ch);  // '('
    std::getline(st, comm, ')');
    p.name = comm;
    for (int i = 0; i < 22 && st >> tok; ++i)
      if (i == 21) p.mem_mb = std::stod(tok) * page_kb / 1024.0;
    procs.push_back(std::move(p));
  }
  closedir(dir);
  if (procs.size() > limit) procs.resize(limit);
  return procs;
}

inline double ReadThermalZone() {
  std::ifstream f("/sys/class/thermal/thermal_zone0/temp");
  if (!f) return std::nan("");
  double milli;
  f >> milli;
  return milli / 1000.0;
}

}  // namespace stacktrace::platform::proclinux
#endif  // __linux__
