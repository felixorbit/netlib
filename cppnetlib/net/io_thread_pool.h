#ifndef CPPNETLIB_NET_IO_THREAD_POOL_H_
#define CPPNETLIB_NET_IO_THREAD_POOL_H_

#include <vector>
#include <memory>
#include <functional>
#include "cppnetlib/core/noncopyable.h"
#include "cppnetlib/core/common_string.h"


namespace cppnetlib {
namespace net {

class EventLoop;
class IOThread;

class IOThreadPool : Noncopyable {
 public:
  IOThreadPool(EventLoop* base_loop, CommonString name);
  ~IOThreadPool();

  using ThreadInitCallback = std::function<void(EventLoop*)>;

  // Create and start threads. 
  void Start(const ThreadInitCallback& cb = ThreadInitCallback());
  // Return a EventLoop in pool.
  EventLoop* GetNextLoop();

  void set_thread_num(int num) { thread_num_ = num; }
  
  bool started() const { return started_; }
  const std::string& name() const { return name_; }

 private:
  EventLoop* base_loop_;

  std::string name_;
  std::vector<std::unique_ptr<IOThread>> threads_;  // all threads
  std::vector<EventLoop*> loops_;  // one loop per thread
  int thread_num_;
  int next_;
  bool started_;
};


} // namespace net
} // namespace cppnetlib

#endif // CPPNETLIB_NET_IO_THREAD_POOL_H_