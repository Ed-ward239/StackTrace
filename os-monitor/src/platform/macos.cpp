// macOS metrics backend — uses sysctl, Mach host statistics, and getifaddrs.
#if defined(__APPLE__)

#include "platform/Metrics.hpp"

#include <ifaddrs.h>
#include <mach/mach.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <sys/mount.h>
#include <sys/sysctl.h>

#include <cmath>
#include <cstring>
#include <string>

namespace stacktrace::platform {
namespace {

template <typename T>
bool Sysctl(const char* name, T& out) {
  size_t len = sizeof(T);
  return sysctlbyname(name, &out, &len, nullptr, 0) == 0;
}

}  // namespace

std::string PlatformName() { return "macOS"; }

CpuSample ReadCpu() {
  static uint64_t prev_used = 0, prev_total = 0;
  CpuSample sample;

  host_cpu_load_info_data_t info;
  mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
  if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO,
                      reinterpret_cast<host_info_t>(&info), &count) != KERN_SUCCESS)
    return sample;

  uint64_t user = info.cpu_ticks[CPU_STATE_USER];
  uint64_t sys = info.cpu_ticks[CPU_STATE_SYSTEM];
  uint64_t nice = info.cpu_ticks[CPU_STATE_NICE];
  uint64_t idle = info.cpu_ticks[CPU_STATE_IDLE];
  uint64_t used = user + sys + nice;
  uint64_t total = used + idle;

  uint64_t du = used - prev_used, dt = total - prev_total;
  if (dt > 0) sample.usage_percent = 100.0 * double(du) / double(dt);
  prev_used = used;
  prev_total = total;
  return sample;
}

MemInfo ReadMemory() {
  MemInfo m;
  uint64_t total = 0;
  if (Sysctl("hw.memsize", total)) m.total_kb = total / 1024;

  vm_size_t page_size = 0;
  host_page_size(mach_host_self(), &page_size);
  vm_statistics64_data_t vm;
  mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
  if (host_statistics64(mach_host_self(), HOST_VM_INFO64,
                        reinterpret_cast<host_info64_t>(&vm), &count) == KERN_SUCCESS) {
    uint64_t free_bytes = uint64_t(vm.free_count) * page_size;
    m.used_kb = (m.total_kb > free_bytes / 1024) ? m.total_kb - free_bytes / 1024 : 0;
  }
  return m;
}

NetInfo ReadNetwork() {
  NetInfo n;
  struct ifaddrs* addrs = nullptr;
  if (getifaddrs(&addrs) != 0) return n;
  for (auto* a = addrs; a; a = a->ifa_next) {
    if (!a->ifa_addr || a->ifa_addr->sa_family != AF_LINK) continue;
    if (std::strcmp(a->ifa_name, "lo0") == 0) continue;
    auto* data = reinterpret_cast<if_data*>(a->ifa_data);
    if (data) {
      n.rx_bytes += data->ifi_ibytes;
      n.tx_bytes += data->ifi_obytes;
    }
  }
  freeifaddrs(addrs);
  return n;
}

DiskInfo ReadDisk(const std::string& mount_point) {
  DiskInfo d;
  struct statfs s {};
  if (statfs(mount_point.c_str(), &s) == 0) {
    d.total_bytes = uint64_t(s.f_blocks) * s.f_bsize;
    d.free_bytes = uint64_t(s.f_bavail) * s.f_bsize;
  }
  return d;
}

std::vector<ProcInfo> ReadProcesses(std::size_t limit) {
  // TODO: enumerate via sysctl(KERN_PROC_ALL) + proc_pidinfo for CPU/mem.
  (void)limit;
  return {};
}

double ReadCpuTemperature() {
  // TODO: read SMC sensor via IOKit (SMCKeys "TC0P"/"TC0D").
  return std::nan("");
}

std::vector<GpioPin> ReadGpio() { return {}; }

}  // namespace stacktrace::platform
#endif  // __APPLE__
