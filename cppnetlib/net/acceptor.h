#ifndef CPPNETLIB_NET_ACCEPTOR_H_
#define CPPNETLIB_NET_ACCEPTOR_H_

#include <functional>
#include "cppnetlib/core/noncopyable.h"
#include "cppnetlib/base/socket.h"
#include "cppnetlib/net/channel.h"

namespace cppnetlib {
namespace net {

class EventLoop;
class InetAddr;

// Create and Listen a socket, accept connection and execute callback.
class Acceptor : Noncopyable {
 public:
  Acceptor(EventLoop* loop, const InetAddr& listen_addr, bool reuse_port);
  ~Acceptor();

  // Main function. To be called by servers.
  void Listen();

  // Configure Channel (callback).
  using ConnCallback = std::function<void (int, const InetAddr&)>;
  void set_accept_callback(const ConnCallback& cb) { accept_callback_ = cb; }

  bool listening() const { return listening_; }

 private:
  void HandleRead();

  EventLoop* loop_;

  Socket listen_socket_;  // indirectly own fd
  Channel listen_channel_;
  ConnCallback accept_callback_;

  bool listening_;

  static const int kNumTryAccept;
};

} // namespace net
} // namespace cppnetlib

#endif // CPPNETLIB_NET_ACCEPTOR_H_