#ifndef CPPNETLIB_NET_TCP_CONNECTION_H_
#define CPPNETLIB_NET_TCP_CONNECTION_H_

#include <memory>
#include <functional>

#include "cppnetlib/core/noncopyable.h"
#include "cppnetlib/base/inet_addr.h"
#include "cppnetlib/base/buffer.h"
#include "cppnetlib/net/context.h"


namespace cppnetlib {
namespace net {

class Socket;
class EventLoop;
class Channel;
class TCPConnection;

using TCPConnectionPtr = std::shared_ptr<TCPConnection>;

using ConnectionCallback = std::function<void(const TCPConnectionPtr&)>;
using MessageCallback = std::function<void(const TCPConnectionPtr&, Buffer*)>;
using CloseCallback = std::function<void(const TCPConnectionPtr&)>;
using TimerCallback = std::function<void()>;

class TCPConnection : Noncopyable, 
                      public std::enable_shared_from_this<TCPConnection> {
 public:
  TCPConnection(EventLoop* loop, const std::string& name, int conn_sockfd, 
                const InetAddr& sock_addr, const InetAddr& peer_addr);
  ~TCPConnection();

  void Send(const void* message, int len);
  void Send(const char* message, int len);
  void Send(const std::string& message);
  void Send(Buffer* buf);

  void Shutdown();

  void AfterEstablished();
  void AfterDestoyed();

  void SetTCPNoDelay(bool on);
  void SetReadState(bool enable);

  void set_connection_callback(const ConnectionCallback& cb) { conn_cb_ = cb; }
  void set_message_callback(const MessageCallback& cb) { message_cb_ = cb; }
  void set_close_callback(const CloseCallback& cb) { close_cb_ = cb; }
  void set_context(Context* context) { context_.reset(context); }

  bool connected() const { return conn_state_ == kConnected; }
  bool disconnected() const { return conn_state_ == kDisconnected; }

  const std::string& name() const { return name_; }
  const InetAddr& sock_addr() const { return sock_addr_; }
  const InetAddr& peer_addr() const { return peer_addr_; }
  EventLoop* get_loop() const { return loop_; }
  Context* mutable_context() { return context_.get(); } 

 private:
  enum ConnState { kDisconnected, kConnecting, kConnected, kDisconnecting };

  void HandleRead();
  void HandleWrite();
  void HandleClose();
  void HandleError();

  void SendInLoop(const std::string& message);
  void SendInLoop(const void* message, size_t len);
  void ShutdownInLoop();
  void SetReadStateInLoop(bool enable);

  void set_state(ConnState state) { conn_state_ = state; }
  

  EventLoop* loop_;
  std::string name_;

  ConnState conn_state_;
  bool reading_;

  std::unique_ptr<Socket> conn_socket_;  // indirectly own fd
  std::unique_ptr<Channel> conn_channel_;

  const InetAddr sock_addr_;
  const InetAddr peer_addr_;

  ConnectionCallback conn_cb_;
  MessageCallback message_cb_;
  CloseCallback close_cb_;

  Buffer rd_buffer_;
  Buffer wt_buffer_;

  std::unique_ptr<Context> context_;
};


} // namespace net
} // namespace cppnetlib

#endif // CPPNETLIB_NET_TCP_CONNECTION_H_