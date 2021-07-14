#ifndef CPPNETLIB_NET_IO_THREAD_H_
#define CPPNETLIB_NET_IO_THREAD_H_

#include <functional>
#include "cppnetlib/core/noncopyable.h"
#include "cppnetlib/core/mutex.h"
#include "cppnetlib/core/condition.h"
#include "cppnetlib/core/thread.h"

namespace cppnetlib {
namespace net {

class EventLoop;

class IOThread : Noncopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;

  IOThread(const ThreadInitCallback& cb = ThreadInitCallback(),
           const std::string& name = std::string());
  ~IOThread();

  EventLoop* Start();

 private:
  void ThreadFunc();

  ThreadInitCallback init_callback_;
  Thread thread_;
  EventLoop* loop_;  // Exposed to other thread. Need to be protected
  bool exiting_;
  
  Mutex mutex_;
  Condition cond_;
};

} // namespace net
} // namespace cppnetlib

#endif // CPPNETLIB_NET_IO_THREAD_H_