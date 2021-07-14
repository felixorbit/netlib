#include "cppnetlib/http/http_context.h"

#include <algorithm>
#include "cppnetlib/base/buffer.h"
#include "cppnetlib/core/logger.h"

namespace cppnetlib {
namespace net {

// public

bool HTTPContext::ParseContext(Buffer* buf) {
  bool ok = true;
  if (state_ == kExpectRequestLine) {
    const char* crlf = buf->FindCRLF();
    if (crlf) {
      if (ProcessRequestLine(buf->peek(), crlf)) {
        buf->RetrieveUntil(crlf + 2);
        state_ = kExpectHeaders;
      } else {
        ok = false;
        LOG_INFO << "HTTPContext::ParseRequest fails to parse"
                 << buf->RetrieveAllAsStr();
      }
    }
  }

  while (state_ == kExpectHeaders) {
    const char* crlf = buf->FindCRLF();
    if (crlf) {
      const char* colon = std::find(buf->peek(), crlf, ':');
      if (colon != crlf) {
        std::string field(buf->peek(), colon);
        ++colon;
        while (colon != crlf && isspace(*colon)) {
          ++colon;
        }
        std::string value(colon, crlf);
        while (!value.empty() && isspace(value[value.size() - 1])) {
          value.resize(value.size() - 1);
        }
        request_.add_header(field, value);
      } else {
        state_ = kGotAll; // abandon body
        // state_ = kExpectBody;
      }
      buf->RetrieveUntil(crlf + 2);
    } else {
      // state_ = kGotAll; // abandon body
      break;
    }
  }

  if (state_ == kExpectBody) {
    // FIXME
  }

  return ok;
}

// private

bool HTTPContext::ProcessRequestLine(const char* begin, const char* end) {
  bool succeed = false;
  // Parse Method
  const char* space = std::find(begin, end, ' ');
  if (space != end && request_.set_method(std::string(begin, space))) {
    begin = space + 1;
    // Parse URL
    space = std::find(begin, end, ' ');
    if (space != end) {
      const char* question = std::find(begin, space, '?');
      if (question != space) {
        request_.set_query(std::string(question, space));
      }
      request_.set_path(std::string(begin, question));
      begin = space + 1;
      // Parse Version
      if ((end - begin == 8) && std::equal(begin, end - 1, "HTTP/1.")) {
        succeed = true;
        if (*(end - 1) == '1')
          request_.set_version(HTTPRequest::kHTTP11);
        else if (*(end - 1) == '0')
          request_.set_version(HTTPRequest::kHTTP10);
        else
          succeed = false;
      }
    }
  }
  return succeed;
}

} // namespace net
} // namespace cppnetlib
