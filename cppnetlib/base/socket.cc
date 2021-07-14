#include "cppnetlib/base/socket.h"

#include <netinet/tcp.h>
#include "cppnetlib/base/inet_addr.h"
#include "cppnetlib/base/sock_ops.h"
#include "cppnetlib/core/logger.h"

namespace cppnetlib {
namespace net{

Socket::~Socket() {
  sockets::Close(sockfd_);
}

// public

void Socket::Listen() {
  sockets::Listen(sockfd_);
}

void Socket::Bind(const InetAddr& local_addr) {
  sockets::Bind(sockfd_, local_addr.sockaddr());
}

int Socket::Accept(InetAddr* peer_addr) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  int connfd = sockets::Accept(sockfd_, &addr);
  if (connfd >= 0) {
    peer_addr->set_sockaddr_in(addr);
  }
  return connfd;
}

void Socket::ShutDownWrite() {
  sockets::ShutDownRD(sockfd_);
}

// Socket options
bool Socket::SetTCPNoDelay(bool on) {
  int val = on ? 1 : 0;
  return ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &val,
                      static_cast<socklen_t>(sizeof(val))) < 0 ? false : true;
}

// avoid TIME_WAIT state
// must be called before bind()
bool Socket::SetReuseAddr(bool on) {
  int val = on ? 1 : 0;
  return ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &val,
                      static_cast<socklen_t>(sizeof(val))) < 0 ? false : true;
}

bool Socket::SetReusePort(bool on) {
  int val = on ? 1 : 0;
  return ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &val,
                      static_cast<socklen_t>(sizeof(val))) < 0 ? false : true;
}

// automatically send probe if no data exchange in 2 hours. 
bool Socket::SetKeepAlive(bool on) {
  int val = on ? 1 : 0;
  return ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &val,
                      static_cast<socklen_t>(sizeof(val))) < 0 ? false : true;
}

} // namespace net
} // namespace cppnetlib