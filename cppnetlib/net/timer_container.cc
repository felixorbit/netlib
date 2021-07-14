#include "cppnetlib/net/timer_container.h"

#include <sys/timerfd.h>
#include <unistd.h>

#include "cppnetlib/core/logger.h"
#include "cppnetlib/net/timer.h"
#include "cppnetlib/net/event_loop.h"
#include "cppnetlib/net/timer_id.h"


// int timerfd_create(int clockid, int flags);
// int timerfd_settime(int fd, int flags,
//                     const struct itimerspec *new_value,
//                     struct itimerspec *old_value);
// int timerfd_gettime(int fd, struct itimerspec *curr_value);
// 
// These system calls create and operate on a timer that delivers timer 
//   expiration notifications via a file descriptor with the advantage that the 
//   file descriptor may be monitored by select(2), poll(2), and epoll(7).


namespace cppnetlib {
namespace net {

namespace detail {

struct timespec TimeIntervalFromNow(TimeAnchor when) {
  int64_t msecs = when.time_micro() - TimeAnchor::Now().time_micro();
  if (msecs < 100) 
    msecs = 100;
  
  struct timespec ts;
  ts.tv_sec = static_cast<time_t>(msecs / TimeAnchor::kMSecsPerSec);
  ts.tv_nsec = static_cast<time_t>(1000 * (msecs % TimeAnchor::kMSecsPerSec));
  return ts;
}

inline bool CmpTimer(const Timer* lhs, const Timer* rhs) {
  return (lhs->expiration()).time_micro() < (rhs->expiration()).time_micro();
}


int CreateTimerFD() {
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0) {
    LOG_SYSFATAL << "Failed in timerfd_create()";
  }
  return timerfd;
}

void ResetTimerFD(int timerfd, TimeAnchor expiration) {
  struct itimerspec new_value;
  struct itimerspec old_value;
  memset(&new_value, 0, sizeof(new_value));
  memset(&old_value, 0, sizeof(old_value));

  new_value.it_value = TimeIntervalFromNow(expiration);
  int ret = ::timerfd_settime(timerfd, 0, &new_value, &old_value);
  if (ret) {
    LOG_ERROR << "timerfd_settimer()";
  }
}

void ReadTimerFD(int timerfd, TimeAnchor now) {
  uint64_t howmany;
  ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
  LOG_TRACE << "TimerContainer::ReadTimerFD " << howmany << " at " 
            << now.ToStr();
  if (n != sizeof(howmany)) {
    LOG_ERROR << "TimerContainer::ReadTimerFD() reads " << n 
              << " bytes instead of 8";
  }
}


inline size_t HeapNodeLeftChild(size_t pos) { return pos + pos + 1; }
inline size_t HeapNodeRightChild(size_t pos) { return pos + pos + 2; }
inline size_t HeapNodeParent(size_t pos) { return pos == 0 ? 0 : ((pos-1)/2); }

const int kInvalidHeapPosition = -1;
const int kTopHeapPosition = 0;
} // namespace detail


TimerContainer::TimerContainer(EventLoop* loop)
    : loop_(loop), timer_list_(), timer_num_(0),
      timerfd_(detail::CreateTimerFD()), 
      timerfd_channel_(loop, timerfd_),
      calling_expired_timers_(false) {
  timerfd_channel_.set_read_callback(
      std::bind(&TimerContainer::HandlerRead, this));
  timerfd_channel_.EnableReading();
}

TimerContainer::~TimerContainer() {
  timerfd_channel_.DisableAll();
  timerfd_channel_.Unsubscribe();
  ::close(timerfd_);
  for (Timer* timer : timer_list_) {
    delete timer;
  }
}

// public

TimerID TimerContainer::AddTimer(TimerCallback cb, TimeAnchor expiration,
                                 double interval) {
  Timer* timer = new Timer(std::move(cb), expiration, interval);
  loop_->RunInLoop(std::bind(&TimerContainer::AddTimerInLoop, this, timer));
  return TimerID(timer, timer->seq_id());
}

void TimerContainer::Cancel(TimerID timerid) {
  loop_->RunInLoop(std::bind(&TimerContainer::CancelInLoop, this, timerid));
}

// private

// Add Timer to heap.
// Reset the expiration as earliest timer
void TimerContainer::AddTimerInLoop(Timer* timer) {
  loop_->AssertInLoopThread();

  TimeAnchor old_earliest;
  if (!IsHeapEmpty()) {
    old_earliest = HeapTop()->expiration();
  }

  HeapAdd(timer);

  if (timer_num_ == 1 || HeapTop()->expiration() < old_earliest) {
    detail::ResetTimerFD(timerfd_, timer->expiration());
  }
}

void TimerContainer::CancelInLoop(TimerID timerid) {
  loop_->AssertInLoopThread();
  Timer* cancel_timer = timerid.timer_;
  if (cancel_timer->InHeap() &&
      static_cast<size_t>(cancel_timer->timer_pos()) < timer_num_) {
    HeapDel(cancel_timer);
    delete cancel_timer;
  }
}

// Execute callback of expired timers.
// Reset the expiration as earliest timer
void TimerContainer::HandlerRead() {
  loop_->AssertInLoopThread();

  TimeAnchor now(TimeAnchor::Now());
  detail::ReadTimerFD(timerfd_, now);
  TimerList expired = GetExpiredTimers(now);

  calling_expired_timers_ = true;
  for (const Timer* expired_timer : expired) {
    expired_timer->Run();
  }
  calling_expired_timers_ = false;

  Reset(expired);
}

std::vector<Timer*> TimerContainer::GetExpiredTimers(TimeAnchor now) {
  assert(timer_list_.size() == timer_num_);
  TimerList expired;
  while (timer_num_ > 0 && HeapTop()->expiration() <= now) {
    expired.push_back(HeapTop());
    HeapPop();
  }
  assert(timer_list_.size() == timer_num_);
  return expired;
}

void TimerContainer::Reset(const TimerList& expired) {
  for (Timer* expired_timer : expired) {
    delete expired_timer;
  }
  if (timer_num_ > 0) {
    TimeAnchor next_expire = HeapTop()->expiration();
    detail::ResetTimerFD(timerfd_, next_expire);
  }
}


// All Timers are managed by a minimum heap (stored in a vector/array).
// The following are interfaces for Minimum Heap.

Timer* TimerContainer::HeapTop() { return timer_list_.front(); }

bool TimerContainer::IsHeapEmpty() { 
  assert(timer_num_ == timer_list_.size());
  return timer_num_ == 0;
}

void TimerContainer::HeapAdd(Timer* timer) {
  if (!timer->InHeap()) {
    timer_list_.push_back(timer);
    timer->set_timer_pos(timer_num_++);
  }
  assert(timer->timer_pos() == static_cast<int>(timer_num_ - 1));
  HeapSiftUp(static_cast<size_t>(timer->timer_pos()));
}

void TimerContainer::HeapDel(Timer* timer) {
  assert(timer_num_ > 0);
  assert(timer->InHeap() && 
         static_cast<size_t>(timer->timer_pos()) < timer_num_);
  assert(timer_num_ == timer_list_.size());

  size_t del_pos = static_cast<size_t>(timer->timer_pos());
  if(--timer_num_ > 0) {
    {
      Timer* tmp = timer_list_[timer_num_];
      timer_list_[timer_num_] = timer_list_[del_pos];
      timer_list_[del_pos] = tmp;
    }
    timer_list_[del_pos]->set_timer_pos(del_pos);
    HeapSiftDown(del_pos);
    HeapSiftUp(del_pos);
  }

  timer_list_.back()->set_timer_pos(detail::kInvalidHeapPosition);
  timer_list_.pop_back();
}

void TimerContainer::HeapPush(Timer* timer) { 
  HeapAdd(timer); 
}

void TimerContainer::HeapPop() {
  assert(timer_num_ > 0);
  assert(timer_num_ == timer_list_.size());
  assert(0 == HeapTop()->timer_pos());

  if (--timer_num_ > 0) {
    {
      Timer* tmp = timer_list_[timer_num_];
      timer_list_[timer_num_] = timer_list_[0];
      timer_list_[0] = tmp;
    }
    timer_list_[0]->set_timer_pos(detail::kTopHeapPosition);
    timer_list_[timer_num_]->set_timer_pos(detail::kInvalidHeapPosition);

    HeapSiftDown(0);
  } else {
    HeapTop()->set_timer_pos(detail::kInvalidHeapPosition);
  }
  timer_list_.pop_back();
}

// Adjust the position of one node to keep heap order.
void TimerContainer::HeapSiftUp(size_t heap_pos) {
  if (heap_pos == 0) return ;

  size_t child_pos = heap_pos;
  size_t parent_pos = detail::HeapNodeParent(heap_pos);

  do {
    if (detail::CmpTimer(timer_list_[child_pos], timer_list_[parent_pos])) {
      {
        Timer* tmp = timer_list_[child_pos];
        timer_list_[child_pos] = timer_list_[parent_pos];
        timer_list_[parent_pos] = tmp;
      }
      timer_list_[child_pos]->set_timer_pos(child_pos);
      timer_list_[parent_pos]->set_timer_pos(parent_pos);

      child_pos = parent_pos;
      parent_pos = detail::HeapNodeParent(child_pos);
    } else {
      break;
    }
  } while (child_pos > 0);
}

void TimerContainer::HeapSiftDown(size_t heap_pos) {
  size_t parent_pos = heap_pos;
  size_t child_pos = detail::HeapNodeLeftChild(parent_pos);

  if (child_pos >= timer_num_) return;

  do {
    if ((1+child_pos) < timer_num_ &&
         (detail::CmpTimer(timer_list_[1+child_pos], timer_list_[child_pos]))) {
      ++child_pos;
    }

    if (detail::CmpTimer(timer_list_[child_pos], timer_list_[parent_pos])) {
      {
        Timer* tmp = timer_list_[child_pos];
        timer_list_[child_pos] = timer_list_[parent_pos];
        timer_list_[parent_pos] = tmp;
      }
      timer_list_[child_pos]->set_timer_pos(child_pos);
      timer_list_[parent_pos]->set_timer_pos(parent_pos);

      parent_pos = child_pos;
      child_pos = detail::HeapNodeLeftChild(parent_pos);
    } else {
      return;
    }
  } while (child_pos < timer_num_);
}


} // namespace net
} // namespace cppnetlib
