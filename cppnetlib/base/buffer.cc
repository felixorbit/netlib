#include "cppnetlib/base/buffer.h"

#include <algorithm>
#include <sys/uio.h>

namespace cppnetlib {
namespace net {

// Definitions of static members outside the class
const size_t Buffer::kPrepend;
const size_t Buffer::kInitSize;
const char Buffer::kCRLF[] = "\r\n";


void Buffer::Retrieve(size_t len) {
  size_t readable = readable_bytes();
  assert(len <= readable);
  if (len < readable)
    r_idx_ += len;
  else
    RetrieveAll();
}

void Buffer::RetrieveUntil(const char* end) {
  assert(end >= r_pos() && end <= w_pos());
  Retrieve(end - r_pos());
}

std::string Buffer::RetrieveAsStr(size_t len) {
  assert(len <= readable_bytes());
  std::string ans(r_pos(), len);
  Retrieve(len);
  return ans;
}


void Buffer::Append(const char* data, size_t len) {
  CheckAdjustSpace(len);
  std::copy(data, data+len, w_pos());
  WIdxForward(len);
}

void Buffer::Prepend(const void* data, size_t len) {
  assert(len <= prependable_bytes());
  r_idx_ -= len;
  const char* data_begin = static_cast<const char*>(data);
  std::copy(data_begin, data_begin + len, buffer_begin() + r_idx_);
}

void Buffer::Swap(Buffer& buf) {
  buffer_.swap(buf.buffer_);
  std::swap(r_idx_, buf.r_idx_);
  std::swap(w_idx_, buf.w_idx_);
}

const char* Buffer::FindCRLF() const {
  return FindCRLF(r_pos());
}

const char* Buffer::FindCRLF(const char* start) const {
  assert(start >= r_pos());
  assert(start <= w_pos());
  const char* crlf_pos = std::search(start, w_pos(), kCRLF, kCRLF + 2);
  return crlf_pos == w_pos() ? nullptr : crlf_pos;
}

// Reads from file to scattered memory blocks.
ssize_t Buffer::ReadFromFD(int fd, int& err_info) {
  char extra_buffer[65536];
  const size_t writeable = writeable_bytes();
  struct iovec vec[2];
  vec[0].iov_base = buffer_begin() + w_idx_;
  vec[0].iov_len = writeable;
  vec[1].iov_base = extra_buffer;
  vec[1].iov_len = sizeof(extra_buffer);

  const ssize_t n = ::readv(fd, vec, 
                            (writeable < sizeof(extra_buffer) ? 2 : 1));
  if (n < 0) {
    err_info = errno;
  } else if (writeable >= static_cast<size_t>(n)) {
    w_idx_ += n;
  } else {
    w_idx_ = buffer_.size();
    Append(extra_buffer, n - writeable);
  }
  return n;
}

// private

void Buffer::AdjustSpace(size_t len) {

  // just cheap resize and no need to rearrange
  if (buffer_.capacity() - w_idx_ > len) {
    buffer_.resize(w_idx_ + len);
  // need to rearrange
  } else if (writeable_bytes() + prependable_bytes() - kPrepend > len) {
    size_t readable = readable_bytes();
    std::copy(buffer_begin() + r_idx_, buffer_begin() + w_idx_, 
              buffer_begin() + kPrepend);
    r_idx_ = kPrepend;
    w_idx_ = r_idx_ + readable;
    assert(readable == readable_bytes());
  // need to resize and rearrange
  } else {
    size_t readable = readable_bytes();
    buffer_.resize(kPrepend + readable + len);
    std::copy(buffer_begin() + r_idx_, buffer_begin() + w_idx_, 
              buffer_begin() + kPrepend);
    r_idx_ = kPrepend;
    w_idx_ = r_idx_ + readable;
    assert(readable == readable_bytes());
  }
}

} // namespace net
} // namespace cppnetlib
