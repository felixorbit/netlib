#ifndef CPPNETLIB_NET_EVENTLOOP_H_
#define CPPNETLIB_NET_EVENTLOOP_H_

#include <atomic>
#include <vector>
#include <memory>
#include <functional>

#include "cppnetlib/core/mutex.h"
#include "cppnetlib/core/time_anchor.h"

namespace cppnetlib {
namespace net {

class Channel;
class EPoller;
class TimerContainer;
class TimerID;

// Control event distribution.
// Multiplexer(EPoller) + EventHandler(Channel)
class EventLoop : Noncopyable {
 public:
  EventLoop();
  ~EventLoop();

  void Loop();
  void Quit();
  void Wakeup();

  using Task = std::function<void()>;
  void RunInLoop(Task cb);
  void QueInLoop(Task cb);
  size_t QueueSize() const;

  // Interface for Channel
  void UpdateChannel(Channel*);
  void RemoveChannel(Channel*);
  bool HasChannel(Channel*);

  // Interface for Timer
  TimerID RunAt(TimeAnchor time, std::function<void()> cb);
  TimerID RunAfter(double delay, std::function<void()> cb);
  void CancelTimer(TimerID timerid);

  bool event_handling() const { return event_handling_; }
  int64_t iteration() const { return iteration_; }
  void AssertInLoopThread() {
    if (!IsInLoopThread())
      AbortNotInLoopThread();
  }
  bool IsInLoopThread() const {
    return thread_id_ == thisthread::tid();
  }

  static EventLoop* GetCurrentThreadLoop();


 private:
  void AbortNotInLoopThread();
  void DoPendingTasks();
  void WakeupRead();

  using ChannelList = std::vector<Channel*>;

  const pid_t thread_id_;
  int64_t iteration_;
  std::atomic_bool quit_;
  bool looping_;
  bool event_handling_;
  bool calling_pending_tasks_;

  std::unique_ptr<EPoller> epoller_;
  ChannelList active_channels_;
  std::unique_ptr<TimerContainer> timer_container_;

  int wakeup_fd_;  // self-created fd for wakeup
  std::unique_ptr<Channel> wakeup_channel_;

  mutable Mutex mutex_;
  std::vector<Task> pending_tasks_;
};

} // namespace net
} // namespace cppnetlib

#endif // CPPNETLIB_NET_EVENTLOOP_H_