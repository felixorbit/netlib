#include "cppnetlib/net/tcp_server.h"
#include "cppnetlib/net/event_loop.h"
#include "cppnetlib/base/inet_addr.h"
#include "cppnetlib/core/thread.h"
#include "cppnetlib/core/logger.h"

#include <unistd.h>
#include <utility>
#include <cstdio>

using namespace cppnetlib;
using namespace cppnetlib::net;
using std::string;
using std::placeholders::_1;
using std::placeholders::_2;

int numThreads = 0;

class EchoServer {
 public:
  EchoServer(EventLoop* loop, const InetAddr& listen_addr)
      : loop_(loop), server_(loop_, listen_addr, "EchoServer") {

    server_.set_thread_num(numThreads);

    server_.set_connection_callback(std::bind(&EchoServer::onConnection, this, _1));
    server_.set_message_callback(std::bind(&EchoServer::onMessage, this, _1, _2));
  }

  void Start() {
    server_.Start();
  }

 private:
  void onConnection(const TCPConnectionPtr& conn) {
    LOG_INFO << conn->sock_addr().GetIPPortStr() << " -> "
             << conn->peer_addr().GetIPPortStr() << " is "
             << (conn->connected() ? "connected" : "down");
    conn->Send("hello\n");
  }

  void onMessage(const TCPConnectionPtr& conn, Buffer* buf) {
    string msg(buf->RetrieveAllAsStr());
    LOG_INFO << conn->name() << " recv " << msg.size() << " bytes";
    if (msg == "exit\n") {
      conn->Send("bye\n");
      conn->Shutdown();
    }
    if (msg == "quit\n") {
      loop_->Quit();
    }
    conn->Send(msg);
  }

  EventLoop* loop_;
  TCPServer server_;
};

int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << getpid() << ", tid = " << thisthread::tid();
  LOG_INFO << "sizeof TCPConnection = " << sizeof(TCPConnection);

  if (argc > 1) {
      numThreads = atoi(argv[1]);
  }

  InetAddr listen_addr(9900, false);

  EventLoop loop;

  EchoServer server(&loop, listen_addr);

  server.Start();
  loop.Loop();
}