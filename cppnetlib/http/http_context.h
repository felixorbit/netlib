#ifndef CPPNETLIB_HTTP_HTTP_CONTEXT_H_
#define CPPNETLIB_HTTP_HTTP_CONTEXT_H_

#include "cppnetlib/http/http_request.h"
#include "cppnetlib/net/context.h"

namespace cppnetlib {
namespace net {

class Buffer;

// Parsing the received information stored in buffer
//  and fill the HTTPRequest
class HTTPContext : public Context {
 public:
  enum HTTPRequestParseState { 
    kExpectRequestLine,
    kExpectHeaders,
    kExpectBody,
    kGotAll,
  };

  HTTPContext() : state_(kExpectRequestLine) {}

  bool ParseContext(Buffer* buf) override;
  bool GotAll() const { return state_ == kGotAll; }
  void Reset() {
    state_ = kExpectRequestLine;
    HTTPRequest dummy;
    request_.swap(dummy);
  }

  const HTTPRequest& request() const { return request_; }
  HTTPRequest& request() { return request_; }

 private:
  bool ProcessRequestLine(const char* begin, const char* end);
  
  HTTPRequestParseState state_;
  HTTPRequest request_;
};

} // namespace net
} // namespace cppnetlib

#endif