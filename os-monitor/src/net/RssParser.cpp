#include "net/RssParser.hpp"

#include <tinyxml2.h>

namespace stacktrace::net {

using namespace tinyxml2;

namespace {
std::string Text(XMLElement* parent, const char* tag) {
  if (!parent) return {};
  XMLElement* e = parent->FirstChildElement(tag);
  if (e && e->GetText()) return e->GetText();
  return {};
}
}  // namespace

std::vector<RssItem> ParseFeed(const std::string& xml) {
  std::vector<RssItem> items;
  XMLDocument doc;
  if (doc.Parse(xml.c_str(), xml.size()) != XML_SUCCESS) return items;

  XMLElement* root = doc.RootElement();
  if (!root) return items;

  // RSS 2.0: <rss><channel><item>...
  if (XMLElement* channel = root->FirstChildElement("channel")) {
    for (XMLElement* it = channel->FirstChildElement("item"); it;
         it = it->NextSiblingElement("item")) {
      items.push_back({Text(it, "title"), Text(it, "link"), Text(it, "pubDate")});
    }
    return items;
  }

  // Atom: <feed><entry>... with <link href="...">
  for (XMLElement* entry = root->FirstChildElement("entry"); entry;
       entry = entry->NextSiblingElement("entry")) {
    RssItem item;
    item.title = Text(entry, "title");
    item.published = Text(entry, "updated");
    if (XMLElement* link = entry->FirstChildElement("link"))
      if (const char* href = link->Attribute("href")) item.link = href;
    items.push_back(std::move(item));
  }
  return items;
}

}  // namespace stacktrace::net
