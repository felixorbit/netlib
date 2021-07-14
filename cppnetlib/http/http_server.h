#ifndef CPPNETLIB_HTTP_HTTP_SERVER_H_
#define CPPNETLIB_HTTP_HTTP_SERVER_H_

#include "cppnetlib/core/noncopyable.h"
#include "cppnetlib/net/tcp_server.h"
#include "cppnetlib/net/event_loop.h"
#include "cppnetlib/base/inet_addr.h"

namespace cppnetlib {
namespace net {

class HTTPRequest;
class HTTPResponse;

class HTTPServer : Noncopyable {
 public:
  using HTTPCallback = std::function<void(const HTTPRequest&, HTTPResponse*)>;
  HTTPServer(EventLoop* loop, const InetAddr& listen_addr, 
             const std::string& name);

  void Start();

  void set_http_callback(const HTTPCallback& cb) { http_callback_ = cb; }
  void set_thread_num(int num) { server_.set_thread_num(num); }

 private:
  void OnConnection(const TCPConnectionPtr& conn);
  void OnMessage(const TCPConnectionPtr& conn, Buffer* buf);
  void OnRequest(const TCPConnectionPtr&, const HTTPRequest&);

  TCPServer server_;
  HTTPCallback http_callback_;
};

} // namespace net
} // namespace cppnetlib

#endif // CPPNETLIB_HTTP_HTTP_SERVER_H_