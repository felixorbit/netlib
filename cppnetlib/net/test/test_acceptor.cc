#include <unistd.h>
#include <cstdio>

#include "cppnetlib/net/acceptor.h"
#include "cppnetlib/net/event_loop.h"
#include "cppnetlib/base/sock_ops.h"
#include "cppnetlib/base/inet_addr.h"

using namespace cppnetlib;
using namespace cppnetlib::net;

void newConnection(int sockfd, const InetAddr& peer_addr) {
  printf("accept a new connection from %s\n", peer_addr.GetIPStr().c_str());
  if (::write(sockfd, "Hello, how are you?\n", 20) < 0) {
    printf("error write.");
  }
  sockets::Close(sockfd);
}

int main() {
  printf("main() pid = %d\n", getpid());

  InetAddr listen_addr(9900);
  EventLoop loop;

  Acceptor acceptor(&loop, listen_addr, false);
  acceptor.set_accept_callback(newConnection);
  acceptor.Listen();

  loop.Loop();
}