# StackTrace (C++) — native dashboard

A self-contained C++/[FTXUI](https://github.com/ArthurSonzogni/FTXUI) rewrite of
the shell prototype. One binary renders every panel natively — no tmux, no
Homebrew tools, zero runtime dependencies. Runs on macOS, Linux, and Raspberry Pi.

> **Status: ready for use on macOS & Linux (Phases 1–4).** All panels are wired
> to live data: system metrics from `/proc`/`sysctl`, a block-digit clock, Yahoo
> Finance quotes, RSS/Atom headlines, and an iCal/CalDAV calendar — the network
> panels each run on their own background thread so the UI never blocks.
> Raspberry Pi support (a dedicated GPIO/thermal panel + ARM cross-compile) is
> **in progress** — see the roadmap in the [root README](../README.md).

## Build

```bash
# macOS
brew install cmake
# Ubuntu / Debian / Raspberry Pi OS
sudo apt install cmake build-essential libcurl4-openssl-dev

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/stacktrace            # q or Esc to quit
```

The first configure downloads FTXUI, nlohmann/json, and tinyxml2 via CMake
`FetchContent` (network required once). `libcurl` is optional — without it the
stock and news panels show placeholder data.

To force the Raspberry Pi backend when cross-compiling:

```bash
cmake -B build -DSTACKTRACE_TARGET_PI=ON
```

## Configuration

Copy [`config.example.json`](config.example.json) to
`~/.config/stacktrace/config.json` and edit. Missing keys fall back to built-in
defaults.

## Source map

```
src/
├── main.cpp              Entry point + 3-row grid layout engine + refresh loop
├── panels/
│   ├── Panel.hpp         Base interface: Render() / Update() / Title()
│   ├── SystemMetrics.*   CPU, RAM, disk, network, temp, process list
│   ├── Clock.*           Block-digit clock (std::chrono)
│   ├── StockTicker.*     Yahoo Finance quotes (own client + worker thread)
│   ├── Calendar.*        Month grid (today highlighted) + upcoming events
│   ├── FileManager.*     Directory browser (std::filesystem)
│   ├── NewsReader.*      RSS/Atom headlines (own client + worker thread)
│   └── Notes.*           Notes-file viewer
├── platform/
│   ├── Metrics.hpp       OS-agnostic metrics interface
│   ├── ProcLinux.hpp     Shared /proc + /sys readers (Linux family)
│   ├── linux.cpp         Generic Linux backend
│   ├── macos.cpp         sysctl + Mach + getifaddrs backend
│   └── raspberrypi.cpp   Linux readers + Pi thermal/GPIO
├── net/
│   ├── HttpClient.*      libcurl wrapper (no-op stub without libcurl)
│   └── RssParser.*       tinyxml2 RSS/Atom parser
├── calendar/
│   └── ICalParser.*      RFC 5545 .ics parser (folding, DATE/DATE-TIME, UTC)
└── config/
    └── Config.*          ~/.config/stacktrace/config.json loader
```

CMake selects exactly one `platform/*.cpp` for the host, so the panel code never
sees an `#ifdef`. The three network panels (markets, news, calendar) each own a
`HttpClient` and a worker thread, so the refresh loop never blocks on I/O.
