#ifndef CPPNETLIB_CORE_LOG_STREAM_H_
#define CPPNETLIB_CORE_LOG_STREAM_H_

#include "cppnetlib/core/fixed_buffer.h"

#include <string>
#include <fmt/format.h>

namespace cppnetlib {

// Maintains a FixedBuffer
//   and provides simple interface (operator<<)
class LogStream : Noncopyable {
 public:
  using Buffer = detail::FixedBuffer<detail::kSmallBuffer>;

  template<typename Plain>
  LogStream& operator<<(Plain v) {
    if (buffer_.available() >= kMaxNumericSize) {
      const char* cptr = fmt::format_to(buffer_.current(), "{}", v);
      buffer_.Add(cptr - buffer_.current());
    }
    return *this;
  }

  LogStream& operator<<(const Buffer&);
  LogStream& operator<<(const void*);
  LogStream& operator<<(double);
  LogStream& operator<<(float);
  LogStream& operator<<(bool v) {
    buffer_.Append(v ? "1" : "0", 1);
    return *this;
  }
  LogStream& operator<<(const char* str) {
    if (str) {
      buffer_.Append(str, strlen(str));
    }
    return *this;
  }
  LogStream& operator<<(const unsigned char* str) {
    return operator<<(reinterpret_cast<const char*>(str));
  }
  LogStream& operator<<(const std::string& v) {
    buffer_.Append(v.c_str(), v.length());
    return *this;
  }
  LogStream& operator<<(std::string&& v) {  // for rvalue reference
    buffer_.Append(v.c_str(), v.length());
    return *this;
  }

  const Buffer& buffer() const { return buffer_; }
  void Append(const char* data, int len) { buffer_.Append(data, len); }
  void ResetBuffer() { buffer_.reset(); }

 private:
  Buffer buffer_;
  static const int kMaxNumericSize = 32;
};

std::string FormatSI(int64_t n);
std::string FormatIEC(int64_t n);

} // namespace cppnetlib

#endif // CPPNETLIB_CORE_LOG_STREAM_H_