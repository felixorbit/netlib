#include "cppnetlib/net/epoller.h"

#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <cassert>

#include "cppnetlib/core/logger.h"
#include "cppnetlib/net/channel.h"

// typedef union epoll_data {
//     void        *ptr;
//     int          fd;
//     uint32_t     u32;
//     uint64_t     u64;
// } epoll_data_t;
// 
// struct epoll_event {
//     uint32_t     events;  /* Epoll events */
//     epoll_data_t data;    /* User data variable */
// };


static_assert(EPOLLIN == POLLIN, 
              "epoll event flags are implemented identical to poll events");
static_assert(EPOLLERR == POLLERR,      
              "epoll event flags are implemented identical to poll events");
static_assert(EPOLLHUP == POLLHUP,      
              "epoll event flags are implemented identical to poll events");
static_assert(EPOLLPRI == POLLPRI,      
              "epoll event flags are implemented identical to poll events");
static_assert(EPOLLOUT == POLLOUT,      
              "epoll event flags are implemented identical to poll events");
static_assert(EPOLLRDHUP == POLLRDHUP,  
              "epoll event flags are implemented identical to poll events");

namespace cppnetlib{
namespace net {

EPoller::EPoller(EventLoop* loop)
    : owner_loop_(loop),
      epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)),
      channel_num_(0),
      event_list_(kInitEventListSize) {
  if (epoll_fd_ < 0) {
    LOG_SYSFATAL << "EPoller::EPoller, error occur";
  }
}

EPoller::~EPoller() { ::close(epoll_fd_); }


// public

void EPoller::Poll(ChannelList* active_channels, int timeout) {
  LOG_TRACE << "fd count: " << channel_num_;

  int num_event = ::epoll_wait(epoll_fd_, &*event_list_.begin(), 
                               static_cast<int>(event_list_.size()),
                               timeout);
  int curr_errno = errno;
  if (num_event < 0) {
    if (curr_errno != EINTR) {
      errno = curr_errno;
      LOG_SYSERR << "EPoller::Poll()";
    }
  } else if (num_event == 0) {
    LOG_TRACE << "timeout and no file descriptor became ready";
  } else {
    LOG_TRACE << num_event << " events happened";
    FillActiveChannels(num_event, active_channels);
    if (event_list_.size() == static_cast<size_t>(num_event)) {
      event_list_.resize(event_list_.size() * 2);
    }
  }
}


void EPoller::UpdateChannel(Channel* channel) {
  owner_loop_->AssertInLoopThread();

  LOG_TRACE << "fd = " << channel->fd() 
            << (channel->in_poller() ? " in EPoller" : " not in EPoller");
  if (!channel->in_poller()) {
    ++channel_num_;
    channel->set_in_poller(true);
    Update(EPOLL_CTL_ADD, channel);
  } else {
    if (channel->IsNoneEvents()) {
      Update(EPOLL_CTL_DEL, channel);
      channel->set_in_poller(false);
      --channel_num_;
    } else {
      Update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EPoller::RemoveChannel(Channel* channel) {
  owner_loop_->AssertInLoopThread();
  LOG_TRACE << "fd = " << channel->fd();
  assert(channel->IsNoneEvents());
  
  if (channel->in_poller()) {
    Update(EPOLL_CTL_DEL, channel);
    channel->set_in_poller(false);
    --channel_num_;
  }
}


// private

// Record events and return all active channels.
void EPoller::FillActiveChannels(int num_event, 
    ChannelList* active_channels) const {
  for (int i = 0; i < num_event; ++i) {
    Channel* channel = static_cast<Channel*>(event_list_[i].data.ptr);
    channel->set_revents(event_list_[i].events);
    active_channels->push_back(channel);
  }
}

// Modify epoll_fd to update channels.
void EPoller::Update(int operation, Channel* channel) {
  struct epoll_event channel_event;
  memset(&channel_event, 0, sizeof(channel_event));
  channel_event.events = channel->events();
  channel_event.data.ptr = channel;
  int fd = channel->fd();

  LOG_TRACE << "epoll_ctl op = " << OpStr(operation) << " fd = " << fd;
  if (::epoll_ctl(epoll_fd_, operation, fd, &channel_event) < 0) {
    (operation == EPOLL_CTL_DEL ? (LOG_SYSERR) : (LOG_SYSFATAL)) 
        << "epoll_ctl op = " << OpStr(operation) << " fd = " << fd;
  }
}

const char* EPoller::OpStr(int op) {
  switch (op)
  {
  case EPOLL_CTL_ADD:
    return "ADD";
  case EPOLL_CTL_DEL:
    return "DEL";
  case EPOLL_CTL_MOD:
    return "MOD";
  default:
    assert(false && "ERROR EPoll OP");
    return "Unkown Operation";
  }
}

} // namespace net
} // namespace cppnetlib
