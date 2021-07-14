#include "cppnetlib/net/acceptor.h"

#include "cppnetlib/core/logger.h"
#include "cppnetlib/base/sock_ops.h"
#include "cppnetlib/base/inet_addr.h"
#include "cppnetlib/net/event_loop.h"

namespace cppnetlib {
namespace net {

const int Acceptor::kNumTryAccept = 10;

Acceptor::Acceptor(EventLoop* loop, const InetAddr& listen_addr, 
                   bool reuse_port)
    : loop_(loop),
      listen_socket_(sockets::Create(listen_addr.family())),
      listen_channel_(loop, listen_socket_.fd()),
      listening_(false) {
  listen_socket_.SetReuseAddr(true);
  listen_socket_.SetReusePort(reuse_port);
  listen_socket_.Bind(listen_addr);

  listen_channel_.set_read_callback(std::bind(&Acceptor::HandleRead, this));
}

Acceptor::~Acceptor() {
  listen_channel_.DisableAll();
  listen_channel_.Unsubscribe();
}

// public 

void Acceptor::Listen() {
  loop_->AssertInLoopThread();
  
  listening_ = true;
  listen_socket_.Listen();
  listen_channel_.EnableReading();
}

// private

// Try to accept N connections every time. (suitable for short connection)
// Execute user callback function.
void Acceptor::HandleRead() {
  loop_->AssertInLoopThread();

  for (int t = 0; t != kNumTryAccept; ++t) {
    InetAddr peer_addr;
    int conn_fd = listen_socket_.Accept(&peer_addr);
    // TODO: file descriptor exhausted situation
    if (conn_fd < 0) {
      if (errno == EAGAIN) {
        LOG_TRACE << "Acceptor::ReadCallback() break for EAGAIN, iter = " 
                  << t + 1;
        break;
      } else {
        LOG_SYSERR << "in Acceptor::ReadCallback";
      }
    } else {
      if (accept_callback_) {
        accept_callback_(conn_fd, peer_addr);
      } else {
        sockets::Close(conn_fd);
      }
    }
  }
}

} // namespace net
} // namespace cppnetlib

