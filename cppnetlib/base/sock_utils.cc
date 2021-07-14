#include "cppnetlib/base/sock_utils.h"
#include <cassert>
#include <cstdio>
#include <cstring>

namespace cppnetlib {
namespace net {
namespace sockets {

int SetSocketAddr(const char* ip, uint16_t port, struct sockaddr_in* addr4) {
  addr4->sin_family = AF_INET;
  addr4->sin_port = HostToNet16(port);
  return ::inet_pton(AF_INET, ip, &addr4->sin_addr);
}

void GetIPStr(char* buf, size_t size, const struct sockaddr_in* addr4) {
  assert(addr4->sin_family == AF_INET);
  ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
}

void GetIPStr(char* buf, size_t size, const struct sockaddr* addr) {
  const struct sockaddr_in* addr4 = CastSockaddrIn(addr);
  ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
}

void GetIPPortStr(char* buf, size_t size, const struct sockaddr_in* addr4) {
  assert(addr4->sin_family == AF_INET);
  GetIPStr(buf, size, addr4);
  size_t end = ::strlen(buf);
  uint16_t port = NetToHost16(addr4->sin_port);
  assert(size > end);
  snprintf(buf+end, size-end, ":%u", port);
}

void GetIPPortStr(char* buf, size_t size, const struct sockaddr* addr) {
  const struct sockaddr_in* addr4 = CastSockaddrIn(addr);
  GetIPPortStr(buf, size, addr4);
}


const struct sockaddr* CastSockaddr(const struct sockaddr_in* addr4) {
  return reinterpret_cast<const struct sockaddr*>(addr4);
}

struct sockaddr* CastSockaddr(struct sockaddr_in* addr4) {
  return reinterpret_cast<struct sockaddr*>(addr4);
}

const struct sockaddr_in* CastSockaddrIn(const struct sockaddr* addr) {
  return reinterpret_cast<const struct sockaddr_in*>(addr);
}

} // namespace sockets
} // namespace net
} // namespace cppnetlib
