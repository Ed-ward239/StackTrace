#include "net/HttpClient.hpp"

#ifdef HAVE_CURL
#include <curl/curl.h>
#endif

namespace stacktrace::net {

#ifdef HAVE_CURL
namespace {
size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
  auto* out = static_cast<std::string*>(userdata);
  out->append(ptr, size * nmemb);
  return size * nmemb;
}
}  // namespace

HttpClient::HttpClient() { handle_ = curl_easy_init(); }

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
