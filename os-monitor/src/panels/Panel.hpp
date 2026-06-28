#pragma once

#include <ftxui/dom/elements.hpp>
#include <string>

namespace stacktrace {

// Base interface every dashboard panel implements.
//
// The layout engine in main.cpp owns a collection of Panels and, on each
// refresh tick, calls Update() (cheap state refresh, may be throttled
// internally) followed by Render() (pure — builds the FTXUI element tree).
class Panel {
 public:
  virtual ~Panel() = default;

  // Build the panel's current visual representation. Must not block.
  virtual ftxui::Element Render() = 0;

  // Refresh internal state from its data source. Called on the refresh
  // thread; implementations should throttle expensive work themselves.
  virtual void Update() = 0;

  // Short title shown in the panel's window border.
  virtual std::string Title() const = 0;
};

}  // namespace stacktrace
