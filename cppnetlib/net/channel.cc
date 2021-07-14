#include "cppnetlib/net/channel.h"

#include <poll.h>
#include <cassert>

#include "cppnetlib/core/logger.h"
#include "cppnetlib/net/event_loop.h"

namespace cppnetlib {
namespace net{

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), in_poller_(false),
      tied_(false), event_handling_(false), added_to_loop_(false) {}

Channel::~Channel() {
  assert(!event_handling_);
  assert(!added_to_loop_);
}

// public

void Channel::HandleEvent() {
  if (tied_) {
    if (tie_.lock()) {
      EventCallback();
    } 
  } else {
    EventCallback();
  }
}

void Channel::Tie(const std::shared_ptr<void>& tie_obj) {
  tie_ = tie_obj;
  tied_ = true;
}

void Channel::Subscribe() {
  added_to_loop_ = true;
  loop_->UpdateChannel(this);
}

void Channel::Unsubscribe() {
  assert(IsNoneEvents());
  added_to_loop_ = false;
  loop_->RemoveChannel(this);
}

// private

void Channel::EventCallback() {
  event_handling_ = true;
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    if (CloseCallback)
      CloseCallback();
  }

  if (revents_ & POLLNVAL) {
    LOG_WARN << "fd " << fd_ << " Channel::HandleEvent() POLLNVAL";
  }

  if (revents_ & (POLLERR | POLLNVAL)) {
    if (ErrorCallback)
      ErrorCallback();
  }

  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (ReadCallback)
      ReadCallback();
  }

  if (revents_ & POLLOUT) {
    if (WriteCallback)
      WriteCallback();
  }
  event_handling_ = false;
}

} // namespace net
} // namespace cppnetlib
