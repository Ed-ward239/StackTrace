// Raspberry Pi metrics backend — generic Linux /proc readers plus Pi-specific
// thermal and GPIO state. Selected by CMake when a Pi is detected.
#if defined(RASPBERRY_PI)

#include "platform/ProcLinux.hpp"

#include <array>
#include <fstream>

#ifdef HAVE_PIGPIO
#include <pigpio.h>
#endif

namespace stacktrace::platform {

std::string PlatformName() { return "Raspberry Pi"; }

// Generic system metrics come straight from the shared Linux readers.
CpuSample ReadCpu() { return proclinux::ReadCpu(); }
MemInfo ReadMemory() { return proclinux::ReadMemory(); }
NetInfo ReadNetwork() { return proclinux::ReadNetwork(); }
DiskInfo ReadDisk(const std::string& mp) { return proclinux::ReadDisk(mp); }
std::vector<ProcInfo> ReadProcesses(std::size_t limit) {
  return proclinux::ReadProcesses(limit);
}

// The Pi exposes the SoC temperature at the standard thermal zone.
double ReadCpuTemperature() { return proclinux::ReadThermalZone(); }

// Sample a handful of commonly used BCM GPIO pins.
std::vector<GpioPin> ReadGpio() {
  static const std::array<int, 8> kPins = {4, 17, 18, 22, 23, 24, 27, 25};
  std::vector<GpioPin> pins;
#ifdef HAVE_PIGPIO
  static bool initialised = (gpioInitialise() >= 0);
  if (initialised) {
    for (int p : kPins) pins.push_back({p, gpioRead(p) == 1});
    return pins;
  }
#endif
  // Fallback: read exported pins via the legacy sysfs interface if present.
  for (int p : kPins) {
    std::ifstream f("/sys/class/gpio/gpio" + std::to_string(p) + "/value");
    if (f) {
      int v = 0;
      f >> v;
      pins.push_back({p, v == 1});
    }
  }
  return pins;
}

}  // namespace stacktrace::platform
#endif  // RASPBERRY_PI
