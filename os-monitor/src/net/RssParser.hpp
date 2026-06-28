#pragma once

#include <string>
#include <vector>

namespace stacktrace::net {

struct RssItem {
  std::string title;
  std::string link;
  std::string published;
};

// Parses an RSS 2.0 or Atom feed document into a flat list of items.
// Returns an empty vector on malformed input.
std::vector<RssItem> ParseFeed(const std::string& xml);

}  // namespace stacktrace::net
