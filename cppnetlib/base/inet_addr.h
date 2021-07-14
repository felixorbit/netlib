#ifndef CPPNETLIB_BASE_INET_ADDR_H_
#define CPPNETLIB_BASE_INET_ADDR_H_

#include <netinet/in.h>
#include "cppnetlib/core/common_string.h"


// /* Structure describing an Internet socket address. */
// struct sockaddr_in {
//     sa_family_t    sin_family; /* address family: AF_INET */
//     uint16_t       sin_port;   /* port in network byte order */
//     struct in_addr sin_addr;   /* internet address */
// };

// /* Internet address. */
// typedef uint32_t in_addr_t;
// struct in_addr {
//     in_addr_t       s_addr;     /* address in network byte order */
// };

namespace cppnetlib {
namespace net {

// [ADT] Socket address for IPv4.
// Maintains ip and port in net endian (aka. big endian)
class InetAddr {
 public:
  // Construct from ip and port.
  InetAddr(CommonString ip_str, uint16_t port);
  // Construct from port. (ip is localhost or any)
  explicit InetAddr(uint16_t port = 0, bool loopback = false);
  // Construct from a origin sockaddr_in
  explicit InetAddr(const struct sockaddr_in& addr) : addr_(addr) {}
  ~InetAddr() = default;

  // Get converted data.
  uint16_t GetPort() const;
  std::string GetIPStr() const;
  std::string GetIPPortStr() const;

  // Access original sockaddr_in data
  const struct sockaddr* sockaddr() const;
  const struct sockaddr_in* sockaddr_in() const;
  void set_sockaddr_in(const struct sockaddr_in&);
  sa_family_t family() const { return addr_.sin_family; }
  uint32_t ip_net() const { return addr_.sin_addr.s_addr; }
  uint16_t port_net() const { return addr_.sin_port; }

 private:
  struct sockaddr_in addr_;
};

} // namespace net
} // namespace cppnetlib

#endif