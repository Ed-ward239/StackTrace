// Linux metrics backend — thin delegation to the shared /proc readers.
#if defined(__linux__) && !defined(RASPBERRY_PI)

#include "platform/ProcLinux.hpp"

namespace stacktrace::platform {

std::string PlatformName() { return "Linux"; }

CpuSample ReadCpu() { return proclinux::ReadCpu(); }
MemInfo ReadMemory() { return proclinux::ReadMemory(); }
NetInfo ReadNetwork() { return proclinux::ReadNetwork(); }
DiskInfo ReadDisk(const std::string& mp) { return proclinux::ReadDisk(mp); }
std::vector<ProcInfo> ReadProcesses(std::size_t limit) {
  return proclinux::ReadProcesses(limit);
}
double ReadCpuTemperature() { return proclinux::ReadThermalZone(); }
std::vector<GpioPin> ReadGpio() { return {}; }

}  // namespace stacktrace::platform
#endif  // __linux__ && !RASPBERRY_PI
