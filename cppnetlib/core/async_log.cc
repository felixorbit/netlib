#include "cppnetlib/core/async_log.h"
#include "cppnetlib/core/time_anchor.h"

namespace cppnetlib {

// public

AsyncLog::AsyncLog(const std::string& basename, off_t rollsize, int flushgap)
    : running_(false), 
      rollsize_(rollsize), 
      flushgap_(flushgap),
      basename_(basename), 
      thread_(std::bind(&AsyncLog::BackendThreadFunc, this), "AsyncLog"),
      latch_(1),
      mutex_(),
      cond_(mutex_),
      major_buf_(new Buffer),
      bak_buf_(new Buffer),
      buffers_() {
  major_buf_ -> bzero();
  bak_buf_ -> bzero();
  buffers_.reserve(16);
}

// Frontend threads write log info to buffer.
// When buffer is full, move it to the array and replace it with a new one.
// *Implemented with double buffering*
void AsyncLog::Append(const char* str_line, int len) {
  MutexLockGuard lock(mutex_);
  if (major_buf_ -> available() > len) {
    major_buf_ -> Append(str_line, len);
  } else {
    buffers_.push_back(std::move(major_buf_));
    if (bak_buf_) {
      major_buf_ = std::move(bak_buf_);
    } else {  // In case of that 2 buffers are full
      major_buf_.reset(new Buffer);
    }
    major_buf_ -> Append(str_line, len);
    cond_.Notify();
  }
}

// private

// Backend log thread is responsible for (1)transfering and (2)storing 
//   log buffers written by frontend.
void AsyncLog::BackendThreadFunc() {
  assert(running_);
  latch_.CountDown();  // synchronize the call of Start() in main thread.

  LogFile output(basename_, rollsize_, false);

  BufferPtr swap_major_buf(new Buffer);
  BufferPtr swap_bak_buf(new Buffer);
  BufferVec swap_buffers;
  swap_major_buf -> bzero();
  swap_bak_buf -> bzero();
  swap_buffers.reserve(16);

  while (running_) {
    assert(swap_major_buf && swap_major_buf -> length() == 0);
    assert(swap_bak_buf && swap_bak_buf -> length() == 0);
    assert(swap_buffers.empty());

    // Log Buffer Access Protected by Mutex
    // 1. When overtime or at least one buffer is full, 
    //   swap frontend buffer array and backend buffer array.
    // 2. Use backend buffers to replace frontend ones.
    {
      MutexLockGuard lock(mutex_);
      
      if (buffers_.empty()) {
        cond_.WaitSecs(flushgap_);
      }

      buffers_.push_back(std::move(major_buf_));
      swap(buffers_, swap_buffers);

      major_buf_ = std::move(swap_major_buf);
      if (!bak_buf_) {
        bak_buf_ = std::move(swap_bak_buf);
      }
    }

    // In case of accumulated too much buffer,
    //   the extra will be discarded.
    if (swap_buffers.size() > 25) {
      char buf[256];
      snprintf(buf, sizeof(buf), 
               "Overmuch accumulated buffers!"
               "Dropped log message at %s.\n", 
               TimeAnchor::Now().FormatMicro().c_str());
      output.Append(buf, static_cast<int>(strlen(buf)));
      swap_buffers.erase(swap_buffers.begin() + 2, swap_buffers.end());
    }

    // Writes buffers to log file
    for (const auto& buffer : swap_buffers) {
      output.Append(buffer -> data(), buffer -> length());
    }

    // Only keeps 2 buffers to reset {swap_major_buf} and {swap_bak_buf}
    if (swap_buffers.size() > 2) {
      swap_buffers.resize(2);
    }
    if (!swap_major_buf) {
      assert(!swap_buffers.empty());
      swap_major_buf = std::move(swap_buffers.back());
      swap_buffers.pop_back();
      swap_major_buf -> reset();
    }
    if (!swap_bak_buf) {
      assert(!swap_buffers.empty());
      swap_bak_buf = std::move(swap_buffers.back());
      swap_buffers.pop_back();
      swap_bak_buf -> reset();
    }

    swap_buffers.clear();
    output.Flush();
  }
  output.Flush();
}


} // namespace cppnetlib