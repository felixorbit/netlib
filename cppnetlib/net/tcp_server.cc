#include "cppnetlib/net/tcp_server.h"

#include "cppnetlib/net/acceptor.h"
#include "cppnetlib/net/event_loop.h"
#include "cppnetlib/base/sock_ops.h"
#include "cppnetlib/core/logger.h"

namespace cppnetlib {

using std::placeholders::_1;
using std::placeholders::_2;

namespace net {

void DefaultConnectionCallback(const TCPConnectionPtr& conn) {
  LOG_TRACE << conn->sock_addr().GetIPPortStr() << " -> "
            << conn->peer_addr().GetIPPortStr() << " is "
            << (conn->connected() ? "connected" : "down");
}

void DefaultMessageCallback(const TCPConnectionPtr&, Buffer* buf) {
  buf->RetrieveAll();
}


TCPServer::TCPServer(EventLoop* loop, const InetAddr& listen_addr, 
                     const std::string& name, bool reuse_port)
    : name_(name), ip_port_(listen_addr.GetIPPortStr()), 
      started_(false), next_conn_id_(1),
      loop_(loop),
      thread_pool_(new IOThreadPool(loop, name_)),
      acceptor_(new Acceptor(loop, listen_addr, reuse_port)),
      conn_cb_(DefaultConnectionCallback),
      message_cb_(DefaultMessageCallback) {

  acceptor_->set_accept_callback(
      std::bind(&TCPServer::AcceptCallback, this, _1, _2));
}

TCPServer::~TCPServer() {
  loop_->AssertInLoopThread();
  LOG_TRACE << "TCPServer::~TCPServer " << name_;

  for (auto& conn_item : conn_map_) {
    TCPConnectionPtr conn(conn_item.second);
    conn_item.second.reset();
    conn->get_loop()->RunInLoop(std::bind(&TCPConnection::AfterDestoyed, conn));
  }
}

// public

void TCPServer::Start() {
  if (!started_.exchange(true)) {  // exchange(): change to new value 
                                   //   and return old value
    
    thread_pool_->Start(thread_init_cb_);

    assert(!acceptor_->listening());
    loop_->RunInLoop(std::bind(&Acceptor::Listen, acceptor_.get()));
  }
}

// private

// After accept a conn_fd:
//   Create a TCPConnection and record in map.
//   Assign an EventLoop
//   Register callback function
void TCPServer::AcceptCallback(int conn_sockfd, const InetAddr& peer_addr) {
  loop_->AssertInLoopThread();

  EventLoop* selected_loop = thread_pool_->GetNextLoop();

  std::string conn_name = name_ + fmt::format("-{}#{}", 
                                              ip_port_, next_conn_id_++);
  LOG_INFO << "TCPServer::AcceptCallback " << conn_name << " from "
           << peer_addr.GetIPPortStr();

  TCPConnectionPtr conn = std::make_shared<TCPConnection>(
      selected_loop, 
      conn_name,
      conn_sockfd,
      InetAddr(sockets::GetSockAddr(conn_sockfd)),
      peer_addr);
  conn_map_[conn_name] = conn;

  conn->set_connection_callback(conn_cb_);
  conn->set_message_callback(message_cb_);
  conn->set_close_callback(std::bind(&TCPServer::CloseCallback, this, _1));
  
  selected_loop->RunInLoop(std::bind(&TCPConnection::AfterEstablished, conn));
}

// Remove TCPConnection in map.
void TCPServer::CloseCallback(const TCPConnectionPtr& conn) {
  loop_->RunInLoop(std::bind(&TCPServer::CloseConnectionInLoop, this, conn));
}

void TCPServer::CloseConnectionInLoop(const TCPConnectionPtr& conn) {
  loop_->AssertInLoopThread();
  LOG_INFO << "TCPServer::RemoveConnectionInLoop" << " - connection " 
           << conn->name();

  size_t n = conn_map_.erase(conn->name());  // use_count: 2 --> 1
  assert(n == 1);

  EventLoop* conn_loop = conn->get_loop();
  conn_loop->QueInLoop(std::bind(&TCPConnection::AfterDestoyed, conn));
  // * Prolong the life of TCPConnection by bind() until AfterDestoyed()
  // * QueInLoop() is necessary. We need to keep channel object 
  //     until finish channel->HandleEvent() 
}


} // namespace net
} // namespace cppnetlib
