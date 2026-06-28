# StackTrace

**Terminal workspace dashboard — shell prototype + C++ evolution**

StackTrace turns a single [Ghostty](https://ghostty.org) terminal window into a
complete developer environment: real-time system metrics, a live clock, a stock
ticker, a calendar agenda, a file manager, an RSS reader, and a notes editor —
all in one beautiful, distraction-free layout.

> *No browser, no distractions — just code, info, and flow. The shell version
> works today. The C++ version owns the process.*

![Dashboard preview](./v1/img/dashboard.png)

---

## Two-phase architecture

| Phase | What it is | Status |
|-------|------------|--------|
| **[v1 — Shell](v1/)** | A `tmux` session orchestrator that launches 8 best-in-class CLI tools into a tiled layout. Single script, Homebrew dependencies. | ✅ Complete — macOS + Ghostty + zsh |
| **[v2 — C++](v2/)** | A self-contained C++/FTXUI binary that renders every panel natively. Zero external tool dependencies. Cross-platform: macOS · Linux · Raspberry Pi. | 🔄 In progress — scaffold builds & runs |

The shell prototype proves the concept. The C++ rewrite removes every runtime
dependency so the whole dashboard ships as one binary that runs anywhere —
your laptop, an Ubuntu server, or a Raspberry Pi.

---

## Layout

```
+─────────────────+───────────────────────+─────────────────+
|   fastfetch     |   tty-clock (center)  |  ticker (stocks)|
+─────────────────+───────────────────────+─────────────────+
|  btop (metrics) |  calcurse (agenda)    |  yazi (files)   |
+─────────────────+───────────────────────+─────────────────+
|    newsboat (RSS feed reader)           |  nvim (notes)   |
+─────────────────────────────────────────+─────────────────+
```

In v2 the `fastfetch` and `btop` panes merge into a single native
`SystemMetrics` panel.

---

## v1 — shell prototype

A single script creates a named `tmux` session, splits the window into 8 panes,
and launches a tool in each.

### Dependencies (macOS / Homebrew)

```bash
brew install fastfetch tty-clock ticker btop calcurse yazi newsboat neovim tmux
```

### Run

```bash
git clone https://github.com/Ed-ward239/Terminal-Dashboard stacktrace
cd stacktrace
./v1/ghostty_dash.sh
```

| Pane | Tool | Shows |
|------|------|-------|
| 0 | `fastfetch` | OS, kernel, shell, uptime, CPU, RAM — with ASCII logo |
| 1 | `tty-clock` | Large centred digital clock |
| 2 | `ticker` | Real-time stock prices, colour-coded gain/loss |
| 3 | `btop` | CPU, RAM, disk I/O, network, process list — live graphs |
| 4 | `calcurse` | Interactive calendar/agenda (CalDAV-capable) |
| 5 | `yazi` | TUI file manager |
| 6 | `newsboat` | RSS feed reader |
| 7 | `nvim` | Notes editor (`~/notes.txt`) |

### Configuration

Sample configs live in [`v1/configs/`](v1/configs/) — copy them into place:

| File | Install to | Controls |
|------|-----------|----------|
| `ticker.yaml` | `~/.config/ticker/ticker.yaml` | Stock watchlist |
| `newsboat_urls` | `~/.newsboat/urls` | RSS subscriptions |
| `.tmux.conf` | `~/.tmux.conf` | Pane borders, status bar, mouse mode |
| `zshrc.snippet` | append to `~/.zshrc` | Auto-launch in Ghostty |

---

## v2 — C++ native rewrite

One compiled binary, zero external tools. See [`v2/README.md`](v2/README.md)
for the full source map and build details.

```bash
# Ubuntu / Debian / Raspberry Pi OS
sudo apt install cmake build-essential libcurl4-openssl-dev
# macOS
brew install cmake

cd v2
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/stacktrace          # q or Esc to quit
```

> **Build target:** v2 is POSIX-only (macOS, Linux, Raspberry Pi). It does not
> build on Windows — develop the C++ phase on a Unix host or WSL.

### Why C++

| v1 limitation | v2 solution | Benefit |
|---------------|-------------|---------|
| 8 Homebrew dependencies | Single compiled binary | Clone and run — zero setup |
| macOS only | POSIX + CMake | macOS + Linux + Raspberry Pi |
| No Pi support | Direct `/proc` + `sysctl` calls | Pi temperature & GPIO readable |
| `btop` rendering overhead | Custom FTXUI panels | Lower CPU usage |
| Layout not programmable | JSON layout config | Customisable arrangement |

### Tech stack

FTXUI (TUI) · `/proc` + `sysctl` (metrics) · libcurl + Yahoo Finance (stocks) ·
libcurl + tinyxml2 (RSS) · `std::chrono` (clock/calendar) · nlohmann/json
(config) · CMake (build) · wiringPi/pigpio (Pi GPIO).

---

## Roadmap (v2)

- [x] **Phase 1 — Foundation:** CMake + FTXUI, 3-row grid layout engine, cross-platform build
- [x] **Phase 2 — System metrics:** platform abstraction (`Metrics.hpp`), CPU/RAM/net/disk reads, live gauges
- [~] **Phase 3 — Clock + ticker:** block-digit clock done; live Yahoo Finance quotes wired via libcurl
- [~] **Phase 4 — RSS + calendar:** RSS/Atom parser done; iCal/CalDAV parsing pending
- [ ] **Phase 5 — Raspberry Pi:** GPIO/thermal panel, ARM cross-compile
- [ ] **Phase 6 — Polish:** JSON layout config, screenshots, demo GIF, prebuilt release binaries

---

## Project structure

```
stacktrace/
├── v1/                     # Shell prototype (complete)
│   ├── ghostty_dash.sh     # Main launch script
│   ├── img/dashboard.png   # Screenshot
│   └── configs/            # ticker.yaml, newsboat_urls, .tmux.conf, zshrc.snippet
├── v2/                     # C++ rewrite (in progress)
│   ├── CMakeLists.txt
│   ├── config.example.json
│   └── src/                # main.cpp, panels/, platform/, net/, config/
└── README.md
```

---

Maintained by Edward Lee.
