#ifndef CPPNETLIB_CORE_ASYNC_LOG_H_
#define CPPNETLIB_CORE_ASYNC_LOG_H_

#include <vector>
#include "cppnetlib/core/log_file.h"
#include "cppnetlib/core/fixed_buffer.h"
#include "cppnetlib/core/thread.h"
#include "cppnetlib/core/countdown_latch.h"

namespace cppnetlib {

class AsyncLog : Noncopyable {
 public:
  AsyncLog(const std::string& basename, off_t rollsize, int flushgap=3);
  ~AsyncLog() {
    if (running_)
      Stop();
  }

  // Interface for frontends logging.
  void Append(const char*, int);

  // Interface for backend log thread control.
  void Start() {
    running_ = true;
    thread_.Start();
    latch_.Wait();  // waits for ThreadFunc start running.
  }
  void Stop() {
    running_ = false;
    cond_.Notify();
    thread_.Join();
  }

 private:
  using Buffer = detail::FixedBuffer<detail::kLargeBuffer>;
  using BufferPtr = std::unique_ptr<Buffer>;
  using BufferVec = std::vector<BufferPtr>;

  // Task of backend log thread
  void BackendThreadFunc();

  std::atomic<bool> running_;

  const off_t rollsize_;  // limits of bytes to roll file
  const int flushgap_;
  const std::string basename_;

  Thread thread_;
  CountdownLatch latch_;  // for backend thread runing
  Mutex mutex_;           // for buffer access
  Condition cond_;

  BufferPtr major_buf_;   // major buffer
  BufferPtr bak_buf_;     // backup buffer
  BufferVec buffers_;     // record full buffers
};

} // namespace cppnetlib


#endif // CPPNETLIB_CORE_ASYNC_LOG_H_