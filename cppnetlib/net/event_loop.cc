#include "cppnetlib/net/event_loop.h"

#include <sys/eventfd.h>
#include <unistd.h>

#include "cppnetlib/net/epoller.h"
#include "cppnetlib/net/timer_container.h"
#include "cppnetlib/base/ignore_sig_pipe.h"
#include "cppnetlib/core/logger.h"
#include "cppnetlib/net/timer_id.h"

namespace cppnetlib {
namespace net {

namespace {
__thread EventLoop* t_loop_in_thread = 0;

const int kEPollWaitTime = 10000;

cppnetlib::net::detail::IgnoreSigPipe ignore_sigpipe_init_obj;
} // namespace

namespace detail {
int CreateEventFD() {
  int event_fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (event_fd < 0) {
    LOG_SYSERR << "Failed in event_fd create";
  }
  return event_fd;
}
} // namespace detail


EventLoop::EventLoop() 
    : thread_id_(cppnetlib::thisthread::tid()), iteration_(0),
      quit_(false), looping_(false), 
      event_handling_(false), calling_pending_tasks_(false), 
      epoller_(new EPoller(this)),
      active_channels_(),
      timer_container_(new TimerContainer(this)),
      wakeup_fd_(detail::CreateEventFD()),
      wakeup_channel_(new Channel(this, wakeup_fd_)),
      mutex_(),
      pending_tasks_() {

  LOG_DEBUG << "EventLoop created " << (void*)this 
            << " in thread " << thread_id_;

  if (t_loop_in_thread) {
    LOG_FATAL << "Another EventLoop " << (void*)t_loop_in_thread
              << " already exists in thread " << thread_id_;
  } else {
    t_loop_in_thread = this;
  }

  // Register wakeup_fd for wakeup need.
  wakeup_channel_->set_read_callback(std::bind(&EventLoop::WakeupRead, this));
  wakeup_channel_->EnableReading();
}

EventLoop::~EventLoop() {
  LOG_DEBUG << "EventLoop " << (void*)this << " of thread " << thread_id_
            << " destructs in thread " << cppnetlib::thisthread::tid();
  wakeup_channel_->DisableAll();
  wakeup_channel_->Unsubscribe();
  ::close(wakeup_fd_);
  t_loop_in_thread = nullptr;
}

// public

// Wait for any events comming and execute callback
// IOThread always works in states:
// 1. blocking at Poll()
// 2. HandleEvent()
// 3. DoPendingTasks()
void EventLoop::Loop() {
  assert(!looping_);
  AssertInLoopThread();

  looping_ = true;
  quit_ = false;
  LOG_TRACE << "Eventloop " << (void*)this << " start looping";

  while (!quit_) {
    active_channels_.clear();
    epoller_->Poll(&active_channels_, kEPollWaitTime);

    ++iteration_;
    event_handling_ = true;
    for (Channel* channel : active_channels_) {
      channel->HandleEvent();
    }
    event_handling_ = false;
    
    DoPendingTasks();
  }

  LOG_TRACE << "EventLoop " << (void*)this << " stop looping";
  looping_ = false;
}

// Wakeup thread blocked at Poll() through a read event on wakeup_fd
// Then get chance to execute DoPendingTasks()
void EventLoop::Wakeup() {
  uint64_t arbitrary = 1;
  ssize_t n = ::write(wakeup_fd_, &arbitrary, sizeof(arbitrary));
  if (n != sizeof(arbitrary)) {
    LOG_ERROR << "EventLoop::Wakeup() writes " << n << " bytes instead of 8";
  }
}

// Just change quit state.  Can be called by other threads.
void EventLoop::Quit() {
  quit_ = true;
  if (!IsInLoopThread()) {  // *Tips: wakeup to quit immediately
    Wakeup();
  }
}

// Execute assigned callback. Can be called by other threads.
void EventLoop::RunInLoop(Task cb) {
  if (IsInLoopThread()) {
    cb();
  } else {
    QueInLoop(cb);
  }
}

void EventLoop::QueInLoop(Task cb) {
  {
    MutexLockGuard lock(mutex_);
    pending_tasks_.push_back(std::move(cb));
  }
  // May be called in pending tasks.
  if (!IsInLoopThread() || calling_pending_tasks_) {
    Wakeup();
  }
}

size_t EventLoop::QueueSize() const {
  MutexLockGuard lock(mutex_);
  return pending_tasks_.size();
}


TimerID EventLoop::RunAt(TimeAnchor time, std::function<void()> cb) {
  return timer_container_->AddTimer(std::move(cb), time, 0.0);
}

TimerID EventLoop::RunAfter(double delay, std::function<void()> cb) {
  int64_t delta = static_cast<int64_t>(delay * TimeAnchor::kMSecsPerSec);
  TimeAnchor time(TimeAnchor::Now().time_micro() + delta);
  return RunAt(time, std::move(cb));
}

void EventLoop::CancelTimer(TimerID timerid) {
  return timer_container_->Cancel(timerid);
}


void EventLoop::UpdateChannel(Channel* channel) {
  assert(channel->owner_loop() == this);
  AssertInLoopThread();
  epoller_->UpdateChannel(channel);
}

void EventLoop::RemoveChannel(Channel* channel) {
  assert(channel->owner_loop() == this);
  AssertInLoopThread();
  epoller_->RemoveChannel(channel);
}

bool EventLoop::HasChannel(Channel* channel) {
  assert(channel->owner_loop() == this);
  AssertInLoopThread();
  return channel->in_poller();
}

EventLoop* EventLoop::GetCurrentThreadLoop() { return t_loop_in_thread; }

// private

void EventLoop::WakeupRead() {
  uint64_t howmany;
  ssize_t n = ::read(wakeup_fd_, &howmany, sizeof(howmany));
  if (n != sizeof(howmany)) {
    LOG_ERROR << "EventLoop::WakeupRead() reads " << n << " bytes instead of 8";
  }
}

void EventLoop::DoPendingTasks() {
  std::vector<Task> tasks;
  calling_pending_tasks_ = true;
  {
    MutexLockGuard lock(mutex_);
    tasks.swap(pending_tasks_); // *Tips: avoid deadlock
  }
  for (const Task& task : tasks) {
    task();
  }
  calling_pending_tasks_ = false;
}

void EventLoop::AbortNotInLoopThread() {
  LOG_FATAL << "AbortNotInLoopThread() EventLoop " << (void*)this 
            << " was created in tid = " << thread_id_ << ", current thread id ="
            << cppnetlib::thisthread::tid();
}

} // namespace net
} // namespace cppnetlib