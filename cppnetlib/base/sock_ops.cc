#include "cppnetlib/base/sock_ops.h"

#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <error.h>

#include "cppnetlib/base/sock_utils.h"
#include "cppnetlib/core/logger.h"

namespace cppnetlib {
namespace net {
namespace sockets {

int SetNonBlockAndCloseOnExec(int sockfd) {
  int flags = ::fcntl(sockfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int ret = ::fcntl(sockfd, F_SETFL, flags);
  if (ret < 0) {
    LOG_ERROR << "sockets::SetNonBlockAndCloseOnExec";
  }

  flags = ::fcntl(sockfd, F_GETFL, 0);
  flags |= FD_CLOEXEC;
  ret = ::fcntl(sockfd, F_SETFL, flags);
  if (ret < 0) {
    LOG_ERROR << "sockets::SetNonBlockAndCloseOnExec";
  }
  return ret;
}

int Create(sa_family_t family) {
  int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 
                        IPPROTO_TCP);  // Since  Linux  2.6.27
  if (sockfd < 0) {
    LOG_SYSFATAL << "sockets::create";
  }
  // SetNonBlockAndCloseOnExec(sockfd);
  return sockfd;
}

void Bind(int sockfd, const struct sockaddr* addr) {
  int ret = (::bind(sockfd, addr, 
             static_cast<socklen_t>(sizeof(struct sockaddr))));
  if (ret < 0) {
    LOG_SYSFATAL << "sockets::bind";
  }
}

void Listen(int sockfd) {
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret < 0) {
    LOG_SYSFATAL << "sockets::listen";
  }
}

int Accept(int sockfd, struct sockaddr* addr) {
  socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
  int connfd = ::accept(sockfd, addr, &addrlen);
  if (connfd < 0) {
    // errors ignored
    int err_info = errno;
    switch (err_info) {
    case EAGAIN:      // no connections are present
    case ECONNABORTED:// connection has been aborted
    case EINTR:       // interrupted by a signal
    case EPROTO:      // protocol error
    case EPERM:       // firewall rules forbid connection
    case EMFILE:      // per-process limit on the number of open fds
      errno = err_info;
      break;
    // unexpected fatal error
    case ENFILE:
    case EFAULT:
      LOG_FATAL << "unexpected error occur when accept" << err_info;
      break;
    default:
      LOG_FATAL << "unkown error occur when accept" << err_info;
      break;
    }
  } else {
    SetNonBlockAndCloseOnExec(connfd);
  }
  return connfd;
}

int Accept(int sockfd, struct sockaddr_in* addr4) {
  return Accept(sockfd, CastSockaddr(addr4));
}

void Close(int sockfd) {
  if (::close(sockfd) < 0) {
    LOG_SYSERR << "sockets::close";
  }
}

void ShutDownWR(int sockfd) {
  if (::shutdown(sockfd, SHUT_WR) < 0) {
    LOG_SYSERR << "sockets::ShutDownWR";
  }
}

void ShutDownRD(int sockfd) {
  if (::shutdown(sockfd, SHUT_RD) < 0) {
    LOG_SYSERR << "sockets::ShutDownRD";
  }
}

struct sockaddr_in GetSockAddr(int sockfd) {
  struct sockaddr_in local_addr;
  memset(&local_addr, 0, sizeof(local_addr));
  socklen_t addrlen = static_cast<socklen_t>(sizeof(local_addr));
  if (::getsockname(sockfd, CastSockaddr(&local_addr), &addrlen) < 0) {
    LOG_SYSERR << "sockets::GetSockAddr";
  }
  return local_addr;
}

struct sockaddr_in GetPeerAddr(int sockfd) {
  struct sockaddr_in peer_addr;
  memset(&peer_addr, 0, sizeof(peer_addr));
  socklen_t addrlen = static_cast<socklen_t>(sizeof(peer_addr));
  if (::getpeername(sockfd, CastSockaddr(&peer_addr), &addrlen) < 0) {
    LOG_SYSERR << "sockets::GetPeerAddr";
  }
  return peer_addr;
}

} // namespace sockets
} // namespace net
} // namespace cppnetlib
