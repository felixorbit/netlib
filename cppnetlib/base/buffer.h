#ifndef CPPNETLIB_BASE_BUFFER_H_
#define CPPNETLIB_BASE_BUFFER_H_

#include <vector>
#include <cassert>
#include "cppnetlib/core/common_string.h"

namespace cppnetlib {
namespace net {

// Read-write cycle
class Buffer {
 public:
  static const size_t kPrepend = 8;
  static const size_t kInitSize = 2040;

  explicit Buffer(size_t init_size = kInitSize)
      : buffer_(kPrepend + init_size), r_idx_(kPrepend), w_idx_(kPrepend) {
    assert(readable_bytes() + writeable_bytes() + kPrepend == buffer_.size());
  }

  // move r_idx_ forward
  void Retrieve(size_t len);
  void RetrieveUntil(const char* end);
  void RetrieveAll() { r_idx_ = kPrepend; w_idx_ = kPrepend; }
  std::string RetrieveAsStr(size_t len);
  std::string RetrieveAllAsStr() { return RetrieveAsStr(readable_bytes()); }
  
  // add data to buffer after w_idx_
  void Append(const char* data, size_t len);
  void Append(const std::string& str) { Append(str.c_str(), str.size()); }
  void Append(const std::string&& str) { Append(str.c_str(), str.size()); }
  void Append(const void* data, size_t len) {
    Append(static_cast<const char*>(data), len);
  }
  
  // add data to buffer before r_idx_
  void Prepend(const void* data, size_t len);
  
  const char* FindCRLF(const char* start) const;
  const char* FindCRLF() const;
  void CheckAdjustSpace(size_t len) {
    if (writeable_bytes() < len) {
      AdjustSpace(len);
    }
    assert(writeable_bytes() >= len);
  }
  void Swap(Buffer& buf);
  ssize_t ReadFromFD(int fd, int& err_info);

  // get value functions
  size_t readable_bytes() const { return w_idx_ - r_idx_; }
  size_t writeable_bytes() const { return buffer_.size() - w_idx_; }
  size_t prependable_bytes() const { return r_idx_; }
  size_t capacity() const { return buffer_.capacity(); }
  const char* r_pos() const { return buffer_begin() + r_idx_; }
  const char* w_pos() const { return buffer_begin() + w_idx_; }
  char* w_pos() { return buffer_begin() + w_idx_; }
  const char* peek() const { return r_pos(); }

 private:
  void WIdxForward(size_t len) {
    assert(len <= writeable_bytes());
    w_idx_ += len;
  }
  void WIdxBackward(size_t len) {
    assert(len <= readable_bytes());
    w_idx_ -= len;
  }
  void AdjustSpace(size_t len);
  
  char* buffer_begin() { return &(*buffer_.begin()); }
  const char* buffer_begin() const { return &(*buffer_.begin()); }

  static const char kCRLF[];

  std::vector<char> buffer_;
  size_t r_idx_;
  size_t w_idx_;
};

} // namespace net
} // namespace cppnetlib

#endif // CPPNETLIB_BASE_BUFFER_H_