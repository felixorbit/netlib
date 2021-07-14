#include "cppnetlib/net/io_thread_pool.h"

#include <fmt/format.h>
#include "cppnetlib/net/event_loop.h"
#include "cppnetlib/net/io_thread.h"

namespace cppnetlib {
namespace net {

IOThreadPool::IOThreadPool(EventLoop* base_loop, CommonString name)
    : base_loop_(base_loop), name_(name.c_str()), thread_num_(0), next_(0), 
      started_(false) {}

IOThreadPool::~IOThreadPool() {}


void IOThreadPool::Start(const ThreadInitCallback& cb) {
  base_loop_->AssertInLoopThread();
  
  assert(!started_);
  started_ = true;

  for (int i = 0; i != thread_num_; ++i) {
    std::unique_ptr<IOThread> io_thread(
        new IOThread(cb, fmt::format("{}{}", name_, i)));
    loops_.push_back(io_thread->Start());
    threads_.push_back(std::move(io_thread));
  }
  if (thread_num_ == 0 && cb) {
    cb(base_loop_);
  }
}

EventLoop* IOThreadPool::GetNextLoop() {
  base_loop_->AssertInLoopThread();
  assert(started_);

  EventLoop* loop = base_loop_;
  if (!loops_.empty()) {
    loop = loops_[next_++];
    if (static_cast<size_t>(next_) >= loops_.size()) {
      next_ = 0;
    }
  }
  return loop;
}


} // namespace net
} // namespace cppnetlib
