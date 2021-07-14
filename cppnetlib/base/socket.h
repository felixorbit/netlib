#ifndef CPPNETLIB_BASE_SOCKET_H_
#define CPPNETLIB_BASE_SOCKET_H_

#include "cppnetlib/core/noncopyable.h"

namespace cppnetlib {
namespace net {

class InetAddr;

class Socket : Noncopyable {
 public:
  explicit Socket(int sockfd) : sockfd_(sockfd) {}
  ~Socket();

  void Listen();
  void Bind(const InetAddr& local_addr);
  int Accept(InetAddr* peer_addr);
  
  void ShutDownWrite();

  int fd() const { return sockfd_; }

  bool SetTCPNoDelay(bool on);
  bool SetReuseAddr(bool on);
  bool SetReusePort(bool on);
  bool SetKeepAlive(bool on);

 private:
  const int sockfd_;
};


} // namespace net
} // namespace cppnetlib

#endif // CPPNETLIB_BASE_SOCKET_H_