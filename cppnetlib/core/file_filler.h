#ifndef CPPNETLIB_CORE_FILE_FILLER_H_
#define CPPNETLIB_CORE_FILE_FILLER_H_

#include "cppnetlib/core/common_string.h"

namespace cppnetlib {

// Use standard I/O operations.
// Manage a file stream and provide write operation in append mode.
class FileFiller {
 public:
  // Opens a stream and set self-defined buffer
  explicit FileFiller(CommonString fname);
  ~FileFiller();

  void Append(const char* str_line, size_t len);
  void Flush();

  off_t written_bytes() const { return written_bytes_; }

 private:
  size_t UnlockedWrite(const char* str_line, size_t len);

  char buffer_[64*1024];  // used for stream buffer
  FILE* fp_;
  off_t written_bytes_;
};

} // namespace cppnetlib

#endif // CPPNETLIB_CORE_FILE_FILLER_H_