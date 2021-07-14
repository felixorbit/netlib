#include "cppnetlib/base/inet_addr.h"

#include "cppnetlib/base/sock_utils.h"
#include "cppnetlib/core/logger.h"

namespace cppnetlib {
namespace net {

InetAddr::InetAddr(CommonString ip_str, uint16_t port) {
  memset(&addr_, 0, sizeof(addr_));
  if (sockets::SetSocketAddr(ip_str.c_str(), port, &addr_) <= 0) {
    LOG_SYSERR << "sockets::FromIpPort in InetAddr(CommonString, uint16_t)";
  }
}

InetAddr::InetAddr(uint16_t port, bool loopback) {
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = sockets::HostToNet32(
      loopback ? INADDR_LOOPBACK : INADDR_ANY);
  addr_.sin_port = sockets::HostToNet16(port);
}

uint16_t InetAddr::GetPort() const {
  return sockets::NetToHost16(port_net());
}

std::string InetAddr::GetIPStr() const {
  char buf[64] = "";
  sockets::GetIPStr(buf, sizeof(buf), &addr_);
  return buf;
}

std::string InetAddr::GetIPPortStr() const {
  char buf[64] = "";
  sockets::GetIPPortStr(buf, sizeof(buf), &addr_);
  return buf;
}

const struct sockaddr* InetAddr::sockaddr() const {
  return sockets::CastSockaddr(&addr_);
}

const struct sockaddr_in* InetAddr::sockaddr_in() const {
  return &addr_;
}

void InetAddr::set_sockaddr_in(const struct sockaddr_in& addr4) {
  addr_ = addr4;
}

} // namespace net
} // namespace cppnetlib