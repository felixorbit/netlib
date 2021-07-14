#ifndef CPPNETLIB_NET_TCP_SERVER_H_
#define CPPNETLIB_NET_TCP_SERVER_H_

#include <atomic>
#include <map>

#include "cppnetlib/net/tcp_connection.h"
#include "cppnetlib/net/io_thread_pool.h"

namespace cppnetlib {
namespace net {

class Acceptor;
class EventLoop;

// Manage all TCPConnection
// 
class TCPServer : Noncopyable {
 public:
  TCPServer(EventLoop* loop, const InetAddr& listen_addr, 
            const std::string& name, bool reuse_port = false);
  ~TCPServer();

  // 1. Start IOThreadPool
  // 2. Let Acceptor begin listening
  void Start();

  // Configure IOThreadPool
  using ThreadInitCallback = std::function<void(EventLoop*)>;
  void set_thread_num(int num) { thread_pool_->set_thread_num(num); }
  void set_thread_init_cb(const ThreadInitCallback& cb) { thread_init_cb_ = cb;}

  // Configure TCPConnection
  void set_connection_callback(const ConnectionCallback& cb) { conn_cb_ = cb; }
  void set_message_callback(const MessageCallback& cb) { message_cb_ = cb; }

  const std::string& ip_port() const { return ip_port_; }
  const std::string& name() const { return name_; }
  EventLoop* get_loop() const { return loop_; }

 private:
  void AcceptCallback(int conn_sockfd, const InetAddr& peer_addr);
  void CloseCallback(const TCPConnectionPtr& conn);
  void CloseConnectionInLoop(const TCPConnectionPtr& conn);
  
  const std::string name_;
  const std::string ip_port_;
  std::atomic_bool started_;
  int next_conn_id_;

  EventLoop* loop_;
  std::unique_ptr<IOThreadPool> thread_pool_;
  std::unique_ptr<Acceptor> acceptor_;
  std::map<std::string, TCPConnectionPtr> conn_map_;

  ThreadInitCallback thread_init_cb_;
  ConnectionCallback conn_cb_;
  MessageCallback message_cb_;
};


} // namespace net
} // namespace cppnetlib

#endif // CPPNETLIB_NET_TCP_SERVER_H_
