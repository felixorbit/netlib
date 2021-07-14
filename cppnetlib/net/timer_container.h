#ifndef CPPNETLIB_NET_TIMER_CONTAINER_H_
#define CPPNETLIB_NET_TIMER_CONTAINER_H_

#include <vector>

#include "cppnetlib/core/noncopyable.h"
#include "cppnetlib/net/channel.h"
#include "cppnetlib/core/time_anchor.h"

namespace cppnetlib {
namespace net {

class EventLoop;
class Timer;
class TimerID;


// Use Channel and timerfd to manage timer events.
// All expired timers share a timerfd.
// Need to reset timerfd when add new timers or handle expired timers.
class TimerContainer : Noncopyable {
 public:
  explicit TimerContainer(EventLoop* loop);
  ~TimerContainer();

  using TimerCallback = std::function<void()>;
  TimerID AddTimer(TimerCallback cb, TimeAnchor expiration, double interval);
  void Cancel(TimerID timerid);

 private:
  using TimerList = std::vector<Timer*>;

  void AddTimerInLoop(Timer* timer);
  void CancelInLoop(TimerID timerid);
  void HandlerRead();
  TimerList GetExpiredTimers(TimeAnchor now);
  void Reset(const TimerList& expired);

  // Implementation aux functions.
  Timer* HeapTop();
  bool IsHeapEmpty();
  void HeapAdd(Timer* timer);
  void HeapDel(Timer* timer);
  void HeapPop();
  void HeapPush(Timer* timer);
  void HeapSiftUp(size_t heap_pos);
  void HeapSiftDown(size_t heap_pos);

  EventLoop* loop_;

  TimerList timer_list_;
  size_t timer_num_;
  const int timerfd_;       // represent expired timers
  Channel timerfd_channel_; // also managed by Channel

  bool calling_expired_timers_;
};



} // namespace net
} // namespace cppnetlib


#endif // CPPNETLIB_NET_TIMER_CONTAINER_H_