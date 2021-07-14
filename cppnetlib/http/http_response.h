#ifndef CPPNETLIB_HTTP_HTTP_RESPONSE_H_
#define CPPNETLIB_HTTP_HTTP_RESPONSE_H_

#include <map>
#include <string>

namespace cppnetlib {
namespace net {

class Buffer;

class HTTPResponse {
 public:
  enum HTTPStatusCode {
    kUnkown,
    kOK = 200,
    kMovedPermanently = 301,
    kBadRequest = 400,
    kNotFound = 404
  };

  explicit HTTPResponse(bool close) : status_code_(kUnkown), 
                                      close_connection_(close) {}

  void AppendToBuffer(Buffer* output) const;

  void AddHeader(const std::string& field, const std::string& value) {
    headers_[field] = value;
  }
  void SetContentType(const std::string& type) { 
    AddHeader("Content-Type", type);
  }

  bool close_connection() const { return close_connection_; }
  void set_status_code(HTTPStatusCode code) { status_code_ = code; }
  void set_status_message(const std::string& m) { status_message_ = m; }
  void set_body(const std::string& body) { body_ = body; }
  void set_close_connection(bool on) { close_connection_ = on; }

 private:
  HTTPStatusCode status_code_;
  bool close_connection_;
  std::string status_message_;
  std::map<std::string, std::string> headers_;
  std::string body_;
};


} // namespace net
} // namespace cppnetlib

#endif // CPPNETLIB_HTTP_HTTP_RESPONSE_H_