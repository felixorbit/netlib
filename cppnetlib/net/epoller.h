#ifndef CPPNETLIB_NET_EPOLLER_H_
#define CPPNETLIB_NET_EPOLLER_H_

#include "cppnetlib/core/noncopyable.h"
#include "cppnetlib/net/event_loop.h"

struct epoll_event;

namespace cppnetlib {
namespace net {

class Channel;

// Multiplexer
// Manage channels and find active ones.
class EPoller : Noncopyable {
 public:
  EPoller(EventLoop* loop);
  ~EPoller();

  using ChannelList = std::vector<Channel*>;
  // Main function. To be called by EventLoop.
  void Poll(ChannelList* active_channels, int timeout);
  // Configure subscribed channel set.
  void UpdateChannel(Channel* channel);
  void RemoveChannel(Channel* channel);

 private:
  static const char* OpStr(int op);
  void Update(int operation, Channel* channel);
  void FillActiveChannels(int num_event, ChannelList* active_channels) const;

  static const int kInitEventListSize = 16;

  EventLoop* owner_loop_;

                  // created by epoll_create() or epoll_create1()
  int epoll_fd_;  // used for all the subsequent calls to the epoll interface.
  int channel_num_;
  std::vector<struct epoll_event> event_list_;
};

} // namespace net
} // namespace cppnetlib

#endif // CPPNETLIB_NET_EPOLLER_H_