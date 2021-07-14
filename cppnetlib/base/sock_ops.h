#ifndef CPPNETLIB_BASE_SOCK_OPS_H_
#define CPPNETLIB_BASE_SOCK_OPS_H_

#include <arpa/inet.h>

namespace cppnetlib {
namespace net {
namespace sockets {

int SetNonBlockAndCloseOnExec(int sockfd);

// Returns a nonblocking socket fd 
int Create(sa_family_t family);
void Bind(int sockfd, const struct sockaddr* addr);
void Listen(int sockfd);
int Accept(int sockfd, struct sockaddr* addr);
int Accept(int sockfd, struct sockaddr_in* addr4);

void Close(int sockfd);
void ShutDownWR(int sockfd);
void ShutDownRD(int sockfd);

struct sockaddr_in GetSockAddr(int sockfd);
struct sockaddr_in GetPeerAddr(int sockfd);

} // namespace sockets
} // namespace net
} // namespace cppnetlib

#endif // CPPNETLIB_BASE_SOCK_OPS_H_