#ifndef CPPNETLIB_CORE_FIXED_BUFFER_H_
#define CPPNETLIB_CORE_FIXED_BUFFER_H_

#include <cstring>
#include <string>

#include "cppnetlib/core/noncopyable.h"

namespace cppnetlib {
namespace detail {

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template<int SIZE>
class FixedBuffer : Noncopyable {
 public :
  FixedBuffer() : cur_(data_) { set_cookie(CookieStart); }
  ~FixedBuffer() { set_cookie(CookieEnd); }

  // set value
  void set_cookie(void (*cookie)()) { cookie_ = cookie; }
  void reset() { cur_ = data_; }
  void bzero() { memset(data_, 0, sizeof(data_)); }

  // get value
  const char* data() const { return data_; }
  char* current() { return cur_; }
  int length() const { return static_cast<int>(cur_ - data_); }
  int available() const { return static_cast<int>(end() - cur_); }

  // operations
  void Add(size_t len) { cur_ += len; }
  void Append(const char* buf, size_t len) {
    if (available() > static_cast<int>(len)) {
      memcpy(cur_, buf, len);
      cur_ += len;
    }
  }
  std::string ToString() const { return std::string(data_, length()); }
  const char* DebugString();

 private:
  const char* end() const { return data_ + sizeof(data_); }

  static void CookieStart();
  static void CookieEnd();

  void (*cookie_)();  // function pointer
  char data_[SIZE];
  char* cur_;
};


} // namespace detail
} // namespace cppnetlib

#endif // CPPNETLIB_CORE_FIXED_BUFFER_H_