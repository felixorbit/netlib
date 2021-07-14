#include "cppnetlib/net/io_thread.h"

#include "cppnetlib/net/event_loop.h"

namespace cppnetlib {
namespace net {

IOThread::IOThread(const ThreadInitCallback& cb, const std::string& name)
    : init_callback_(cb),
      thread_(std::bind(&IOThread::ThreadFunc, this), name), 
      loop_(nullptr), exiting_(false), 
      mutex_(), cond_(mutex_) {}

IOThread::~IOThread() {
  exiting_ = true;
  if (loop_) {
    loop_->Quit();
    thread_.Join();
  }
}

// public

// Start a thread and wait for a ready EventLoop
EventLoop* IOThread::Start() {
  thread_.Start();

  EventLoop* loop = nullptr;
  {
    MutexLockGuard lock(mutex_);
    while (loop_ == nullptr) {
      cond_.Wait();
    }
    loop = loop_;
  }

  return loop;
}

// private

// Main task is executing Loop()
void IOThread::ThreadFunc() {
  EventLoop loop;

  if (init_callback_)
    init_callback_(&loop);

  {
    MutexLockGuard lock(mutex_);
    loop_ = &loop;
    cond_.Notify();
  }

  loop.Loop();

  MutexLockGuard lock(mutex_);
  loop_ = nullptr;
}

} // namespace net
} // namespace cppnetlib
