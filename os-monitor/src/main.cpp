// StackTrace (C++) — self-contained terminal workspace dashboard.
//
// Composes the panels into a 3-row grid with FTXUI, refreshes their state on
// a background thread, and redraws on each tick. Quit with 'q' or Esc.

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "config/Config.hpp"
#include "panels/Calendar.hpp"
#include "panels/Clock.hpp"
#include "panels/FileManager.hpp"
#include "panels/NewsReader.hpp"
#include "panels/Notes.hpp"
#include "panels/Panel.hpp"
#include "panels/StockTicker.hpp"
#include "panels/SystemMetrics.hpp"
#include "platform/Metrics.hpp"

using namespace ftxui;
using namespace stacktrace;

int main() {
  const Config cfg = Config::Load();

  // Own the panels; `panels` is a flat view used by the refresh loop. The
  // network panels (markets, calendar, news) each own a libcurl client and a
  // background worker thread, so they never block this refresh loop.
  auto metrics = std::make_unique<SystemMetricsPanel>();
  auto clk = std::make_unique<ClockPanel>();
  auto markets = std::make_unique<StockTickerPanel>(cfg.tickers);
  auto calendar =
      std::make_unique<CalendarPanel>(Config::ExpandHome(cfg.calendar_source));
  auto files = std::make_unique<FileManagerPanel>(Config::ExpandHome("~/"));
  auto news = std::make_unique<NewsReaderPanel>(cfg.rss_feeds);
  auto notes = std::make_unique<NotesPanel>(Config::ExpandHome(cfg.notes_file));

  std::vector<Panel*> panels = {metrics.get(), clk.get(),  markets.get(),
                                calendar.get(), files.get(), news.get(),
                                notes.get()};

  // Guards panel state shared between the refresh thread and the render loop.
  std::mutex mtx;

  auto cell = [](Panel* p) {
    return window(text(" " + p->Title() + " ") | bold, p->Render() | flex);
  };

  auto screen = ScreenInteractive::Fullscreen();

  auto layout = Renderer([&] {
    std::lock_guard<std::mutex> lk(mtx);
    auto row1 = hbox({cell(metrics.get()) | flex, cell(clk.get()) | flex,
                      cell(markets.get()) | flex});
    auto row2 = hbox({cell(calendar.get()) | flex, cell(files.get()) | flex,
                      cell(news.get()) | flex});
    auto footer = hbox({
        text(" StackTrace ") | bold | bgcolor(Color::Blue),
        text(" " + platform::PlatformName()) | dim,
        filler(),
        text("q to quit ") | dim,
    });
    return vbox({row1 | flex, row2 | flex, cell(notes.get()) | flex, footer});
  });

  auto component = CatchEvent(layout, [&](const Event& e) {
    if (e == Event::Character('q') || e == Event::Escape) {
      screen.Exit();
      return true;
    }
    return false;
  });

  std::atomic<bool> running{true};
  std::thread refresher([&] {
    while (running) {
      {
        std::lock_guard<std::mutex> lk(mtx);
        for (auto* p : panels) p->Update();
      }
      screen.PostEvent(Event::Custom);
      std::this_thread::sleep_for(std::chrono::milliseconds(cfg.refresh_rate_ms));
    }
  });

  screen.Loop(component);

  running = false;
  refresher.join();
  return 0;
}
