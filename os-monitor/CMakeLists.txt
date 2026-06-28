cmake_minimum_required(VERSION 3.20)
project(stacktrace VERSION 2.0.0 LANGUAGES CXX)

# ───────────────────────── Standard ─────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Use GNU extensions (gnu++17) so glibc exposes POSIX additions to the standard
# C headers — e.g. localtime_r in <ctime>, getifaddrs feature guards — which
# strict -std=c++17 hides behind __STRICT_ANSI__.
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ───────────────────── Platform detection ───────────────────
# Pick exactly one platform implementation of the metrics layer so we
# never get duplicate symbols. A Raspberry Pi is just Linux with extras,
# so it is detected first and overrides the generic Linux path.
option(STACKTRACE_TARGET_PI "Force the Raspberry Pi metrics backend" OFF)

if(NOT STACKTRACE_TARGET_PI AND EXISTS "/proc/device-tree/model")
  file(READ "/proc/device-tree/model" _dt_model)
  if(_dt_model MATCHES "Raspberry Pi")
    set(STACKTRACE_TARGET_PI ON)
  endif()
endif()

if(APPLE)
  set(PLATFORM_SRC src/platform/macos.cpp)
  message(STATUS "StackTrace platform backend: macOS (sysctl/IOKit)")
elseif(STACKTRACE_TARGET_PI)
  set(PLATFORM_SRC src/platform/raspberrypi.cpp)
  add_compile_definitions(RASPBERRY_PI=1)
  message(STATUS "StackTrace platform backend: Raspberry Pi (/proc + /sys + GPIO)")
elseif(UNIX)
  set(PLATFORM_SRC src/platform/linux.cpp)
  message(STATUS "StackTrace platform backend: Linux (/proc + /sys)")
else()
  message(FATAL_ERROR "StackTrace targets POSIX systems (macOS, Linux, Raspberry Pi).")
endif()

# ───────────────────── Third-party deps ─────────────────────
include(FetchContent)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
  GIT_TAG v5.0.0)
FetchContent_MakeAvailable(ftxui)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3)
FetchContent_MakeAvailable(json)

FetchContent_Declare(tinyxml2
  GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
  GIT_TAG 10.0.0)
set(tinyxml2_BUILD_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(tinyxml2)

# libcurl is optional — network panels degrade to cached/sample data without it.
find_package(CURL QUIET)

# ───────────────────────── Target ───────────────────────────
add_executable(stacktrace
  src/main.cpp
  src/panels/SystemMetrics.cpp
  src/panels/Clock.cpp
  src/panels/StockTicker.cpp
  src/panels/Calendar.cpp
  src/panels/FileManager.cpp
  src/panels/NewsReader.cpp
  src/panels/Notes.cpp
  src/net/HttpClient.cpp
  src/net/RssParser.cpp
  src/calendar/ICalParser.cpp
  src/config/Config.cpp
  ${PLATFORM_SRC})

target_include_directories(stacktrace PRIVATE src)

target_link_libraries(stacktrace PRIVATE
  ftxui::screen
  ftxui::dom
  ftxui::component
  nlohmann_json::nlohmann_json
  tinyxml2)

if(CURL_FOUND)
  target_link_libraries(stacktrace PRIVATE CURL::libcurl)
  target_compile_definitions(stacktrace PRIVATE HAVE_CURL=1)
  message(STATUS "libcurl found — live network panels enabled")
else()
  message(WARNING "libcurl not found — stock/RSS panels will show sample data")
endif()

if(STACKTRACE_TARGET_PI)
  # Optional Pi GPIO library; pigpio or wiringPi if present.
  find_library(PIGPIO_LIB pigpio)
  if(PIGPIO_LIB)
    target_link_libraries(stacktrace PRIVATE ${PIGPIO_LIB})
    target_compile_definitions(stacktrace PRIVATE HAVE_PIGPIO=1)
  endif()
endif()

find_package(Threads REQUIRED)
target_link_libraries(stacktrace PRIVATE Threads::Threads)
