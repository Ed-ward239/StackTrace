#pragma once

#include <optional>
#include <string>

namespace stacktrace::net {

// Minimal synchronous HTTP GET wrapper around libcurl.
//
// When the project is built without libcurl (HAVE_CURL undefined), Get()
// returns std::nullopt so callers transparently fall back to sample data.
class HttpClient {
 public:
  HttpClient();
  ~HttpClient();

  HttpClient(const HttpClient&) = delete;
  HttpClient& operator=(const HttpClient&) = delete;

  // Returns the response body, or nullopt on failure / no-network build.
  std::optional<std::string> Get(const std::string& url,
                                 long timeout_seconds = 10);

 private:
  void* handle_ = nullptr;  // CURL* (opaque to avoid leaking the header)
};

}  // namespace stacktrace::net
