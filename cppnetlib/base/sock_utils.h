#ifndef CPPNETLIB_BASE_SOCK_UTILS_H_
#define CPPNETLIB_BASE_SOCK_UTILS_H_

#include <arpa/inet.h>
#include <endian.h>

namespace cppnetlib {
namespace net {
namespace sockets {

// Wrapper functions of endian conversion

// net to host
inline uint16_t NetToHost16(uint16_t net16) { return be16toh(net16); }
inline uint32_t NetToHost32(uint32_t net32) { return be32toh(net32); }
inline uint64_t NetToHost64(uint64_t net64) { return be64toh(net64); }

// host to net
inline uint16_t HostToNet16(uint16_t host16) { return htobe16(host16); }
inline uint32_t HostToNet32(uint32_t host32) { return htobe32(host32); }
inline uint64_t HostToNet64(uint64_t host64) { return htobe64(host64); }

// Functions that directly work with original sockaddr_in or sockaddr

// Returns converted sockaddr_in or sockaddr struct.
const struct sockaddr* CastSockaddr(const struct sockaddr_in* addr4);
struct sockaddr* CastSockaddr(struct sockaddr_in* addr4);
const struct sockaddr_in* CastSockaddrIn(const struct sockaddr* addr);

// Returns sockaddr_in struct stored in parameter addr4.
int SetSocketAddr(const char* ip, uint16_t port, struct sockaddr_in* addr4);

// Returns ip string stored in parameter buf.
// Accepts sockaddr_in or sockaddr struct.
void GetIPStr(char* buf, size_t size, const struct sockaddr_in* addr4);
void GetIPStr(char* buf, size_t size, const struct sockaddr* addr);

// Returns ip:port string stored in parameter buf.
// Accepts sockaddr_in or sockaddr struct.
void GetIPPortStr(char* buf, size_t size, const struct sockaddr_in* addr4);
void GetIPPortStr(char* buf, size_t size, const struct sockaddr* addr);

} // namespace sockets
} // namespace net
} // namespace cppnetlib

#endif // CPPNETLIB_BASE_SOCK_UTILS_H_