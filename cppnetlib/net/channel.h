#ifndef CPPNETLIB_NET_CHANNEL_H_
#define CPPNETLIB_NET_CHANNEL_H_

#include <functional>
#include <memory>

#include "cppnetlib/core/noncopyable.h"

namespace cppnetlib {
namespace net {

class EventLoop;

// Manage file descriptor, events and callback.
// But it neither owns file descriptor nor needs to close it.
// 
// Other higher level facilities: 
// * Acceptor(listen_fd)
// * TCPConnection(conn_fd)
// * TimerContainer(timerfd)
// The lifetime of channel object is decided by its owner class.
class Channel : Noncopyable {
 public:
  Channel(EventLoop* loop, int fd);
  ~Channel();

  // Main function. To be called by EventLoop.
  void HandleEvent();

  void Tie(const std::shared_ptr<void>&);

  // Configure events of interest and subscribe/unsubscribe to Multiplexer.
  // Events: None / Read / Write
  bool IsNoneEvents() const { return events_ == kNoneEvent; }
  bool IsReadingEvents() const { return events_ & kReadEvent; }
  bool IsWritingEvents() const { return events_ & kWriteEvent; }

  void Subscribe();
  void Unsubscribe();
  void EnableReading() { 
    events_ |= kReadEvent; 
    Subscribe();
  }
  void EnableWriting() {
    events_ |= kWriteEvent;
    Subscribe();
  }
  void DisableReading() {
    events_ &= ~kReadEvent;
    Subscribe();
  }
  void DisableWriting() {
    events_ &= ~kWriteEvent;
    Subscribe();
  }
  void DisableAll() {
    events_ = kNoneEvent;
    Subscribe();
  }

  // Configure event callback functions.
  using Callback = std::function<void()>;
  void set_read_callback(Callback cb) { ReadCallback = std::move(cb); }
  void set_write_callback(Callback cb) { WriteCallback = std::move(cb); }
  void set_close_callback(Callback cb) { CloseCallback = std::move(cb); }
  void set_error_callback(Callback cb) { ErrorCallback = std::move(cb); }

  void set_revents(int revents) { revents_ = revents; }
  void set_in_poller(bool in_poller) { in_poller_ = in_poller; }

  int fd() const { return fd_; }
  int events() const { return events_; }
  bool in_poller() const { return in_poller_; }
  EventLoop* owner_loop() { return loop_; }


 private:
  void EventCallback();
  Callback ReadCallback;
  Callback WriteCallback;
  Callback CloseCallback;
  Callback ErrorCallback;

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop* loop_;

  const int fd_;
  int events_;  // events we cared
  int revents_; // 

  bool in_poller_;
  std::weak_ptr<void> tie_;
  bool tied_;
  bool event_handling_;
  bool added_to_loop_;
};


} // namespace net
} // namespace cppnetlib

#endif // CPPNETLIB_NET_CHANNEL_H_