#include "net/HttpClient.hpp"

#ifdef HAVE_CURL
#include <curl/curl.h>

#include <mutex>
#endif

namespace stacktrace::net {

#ifdef HAVE_CURL
namespace {
size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
  auto* out = static_cast<std::string*>(userdata);
  out->append(ptr, size * nmemb);
  return size * nmemb;
}

// curl_global_init() is not thread-safe, so run it exactly once on the first
// HttpClient construction (panels build their clients on the main thread before
// starting workers). Each HttpClient owns its own easy handle, used by a single
// worker thread, so handles are never shared across threads.
void EnsureGlobalInit() {
  static std::once_flag flag;
  std::call_once(flag, [] { curl_global_init(CURL_GLOBAL_DEFAULT); });
}
}  // namespace

HttpClient::HttpClient() {
  EnsureGlobalInit();
  handle_ = curl_easy_init();
}

HttpClient::~HttpClient() {
  if (handle_) curl_easy_cleanup(static_cast<CURL*>(handle_));
}

std::optional<std::string> HttpClient::Get(const std::string& url,
                                           long timeout_seconds) {
  if (!handle_) return std::nullopt;
  auto* curl = static_cast<CURL*>(handle_);
  std::string body;
  curl_easy_reset(curl);
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_seconds);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "stacktrace/2.0");
  if (curl_easy_perform(curl) != CURLE_OK) return std::nullopt;
  return body;
}

#else  // ── built without libcurl ──

HttpClient::HttpClient() = default;
HttpClient::~HttpClient() = default;

std::optional<std::string> HttpClient::Get(const std::string&, long) {
  return std::nullopt;
}

#endif  // HAVE_CURL

}  // namespace stacktrace::net
