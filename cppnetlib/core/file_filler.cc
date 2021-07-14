#include "cppnetlib/core/file_filler.h"

#include <cassert>
#include "cppnetlib/core/logger.h"

namespace cppnetlib {

// public

// Opens with append mode. 'e' with O_CLOEXEC flag
FileFiller::FileFiller(CommonString fname) 
    : fp_(fopen(fname.c_str(), "ae")), written_bytes_(0) {
  assert(fp_);
  setbuffer(fp_, buffer_, sizeof(buffer_));  // alias for call to setvbuf()
}

FileFiller::~FileFiller() { fclose(fp_); }

// Writes specified num of bytes.
void FileFiller::Append(const char* str_line, size_t len) {
  size_t written_n = UnlockedWrite(str_line, len);
  size_t residual_n = len - written_n;

  while (residual_n > 0) {
    size_t again_n = UnlockedWrite(str_line + written_n, residual_n);
    if (again_n == 0) {
      int err = ferror(fp_);
      if (err)
        fprintf(stderr, "FileFiller::Append() failed %s\n", strerror_tl(err));
      break;
    }
    written_n += again_n;
    residual_n = len - written_n;
  }
  written_bytes_ += len;
}

// Force the buffer out early.
void FileFiller::Flush() { fflush(fp_); }

// private

size_t FileFiller::UnlockedWrite(const char* str_line, size_t len) {
  return fwrite_unlocked(str_line, 1, len, fp_);  // fwrite_unlocked():
}                                                 //   have the same behavior
                                                  //   with fwrite(). 
                                                  //   but not thread safe.


} // namespace cppnetlib