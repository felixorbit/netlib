#include "cppnetlib/net/tcp_connection.h"

#include <unistd.h>

#include "cppnetlib/net/channel.h"
#include "cppnetlib/core/logger.h"
#include "cppnetlib/base/socket.h"
#include "cppnetlib/net/event_loop.h"

namespace cppnetlib {
namespace net {

TCPConnection::TCPConnection(EventLoop* loop, const std::string& name, 
                             int conn_sockfd, const InetAddr& sock_addr, 
                             const InetAddr& peer_addr)
    : loop_(loop), name_(name), conn_state_(kConnecting), reading_(true),
      conn_socket_(new Socket(conn_sockfd)),
      conn_channel_(new Channel(loop, conn_sockfd)),
      sock_addr_(sock_addr),
      peer_addr_(peer_addr) {
  conn_channel_->set_read_callback(std::bind(&TCPConnection::HandleRead, this));
  conn_channel_->set_write_callback(std::bind(&TCPConnection::HandleWrite, this));
  conn_channel_->set_close_callback(std::bind(&TCPConnection::HandleClose, this));
  conn_channel_->set_error_callback(std::bind(&TCPConnection::HandleError, this));

  LOG_DEBUG << "TCPConnection::TCPConnection " << name_ << " fd = " 
            << conn_socket_->fd();
  conn_socket_->SetKeepAlive(true);
}

TCPConnection::~TCPConnection() {
  LOG_DEBUG << "TCPConnection::~TCPConnection " << name_ << " fd = "
            << conn_channel_->fd() << " state = " << conn_state_;
  assert(conn_state_ == kDisconnected);
}

// public

void TCPConnection::Send(const void* message, int len) {
  Send(std::string(static_cast<const char*>(message), len));
}

void TCPConnection::Send(const char* message, int len) {
  Send(std::string(message, len));
}

void TCPConnection::Send(const std::string& message) {
  if (conn_state_ == kConnected) {
    if (loop_->IsInLoopThread()) {
      SendInLoop(message);
    } else {
      void(TCPConnection::*fp)(const std::string&) = &TCPConnection::SendInLoop;
      loop_->RunInLoop(std::bind(fp, this, message));
    }
  }
}

void TCPConnection::Send(Buffer* buf) {
  if (conn_state_ == kConnected) {
    if (loop_->IsInLoopThread()) {
      SendInLoop(buf->r_pos(), buf->readable_bytes());
      buf->RetrieveAll();
    } else {
      void(TCPConnection::*fp)(const std::string&) = &TCPConnection::SendInLoop;
      loop_->RunInLoop(std::bind(fp, this, buf->RetrieveAllAsStr()));
    }
  }
}

void TCPConnection::Shutdown() {
  if (conn_state_ == kConnected) {
    set_state(kDisconnecting);
    loop_->RunInLoop(std::bind(&TCPConnection::ShutdownInLoop, this));
  }
}

void TCPConnection::AfterEstablished() {
  loop_->AssertInLoopThread();

  assert(conn_state_ == kConnecting);
  set_state(kConnected);

  conn_channel_->Tie(shared_from_this());
  conn_channel_->EnableReading();

  conn_cb_(shared_from_this());
}

void TCPConnection::AfterDestoyed() {
  loop_->AssertInLoopThread();

  if (conn_state_ == kConnected) {
    set_state(kDisconnected);
    conn_channel_->DisableAll();
    conn_cb_(shared_from_this());
  }

  conn_channel_->Unsubscribe();  // avoid dangling pointer to channel
}

void TCPConnection::SetTCPNoDelay(bool on) { 
  conn_socket_->SetTCPNoDelay(on); 
}

void TCPConnection::SetReadState(bool enable) {
  loop_->RunInLoop(std::bind(&TCPConnection::SetReadStateInLoop, this, enable));
}

// private

void TCPConnection::SendInLoop(const std::string& message) {
  SendInLoop(message.c_str(), message.size());
}

void TCPConnection::SendInLoop(const void* message, size_t len) {
  loop_->AssertInLoopThread();
  if (conn_state_ == kDisconnected) {
    LOG_WARN << "try to send to disconnected conn_fd, give up";
    return;
  }
  wt_buffer_.Append(static_cast<const char*>(message), len);
  if (!conn_channel_->IsWritingEvents()) {
    conn_channel_->EnableWriting();
  }
}

void TCPConnection::ShutdownInLoop() {
  loop_->AssertInLoopThread();
  if (!conn_channel_->IsWritingEvents()) {
    conn_socket_->ShutDownWrite();
  }
}

void TCPConnection::SetReadStateInLoop(bool enable) {
  loop_->AssertInLoopThread();
  if (enable) {
    if (!reading_ || !conn_channel_->IsReadingEvents()) {
      conn_channel_->EnableReading();
      reading_ = true;
    }
  } else {
    if (reading_ || conn_channel_->IsReadingEvents()) {
      conn_channel_->DisableReading();
      reading_ = false;
    }
  }
}


void TCPConnection::HandleRead() {
  loop_->AssertInLoopThread();

  int err_info = 0;
  ssize_t n = rd_buffer_.ReadFromFD(conn_channel_->fd(), err_info);

  if (n > 0) {
    message_cb_(shared_from_this(), &rd_buffer_);
  } else if (n == 0) {
    HandleClose();
  } else {
    errno = err_info;
    LOG_SYSERR << "TCPConnection::HandleRead";
    HandleError();
  }
}

void TCPConnection::HandleWrite() {
  loop_->AssertInLoopThread();
  
  if (conn_channel_->IsWritingEvents()) {
    ssize_t n = ::write(conn_channel_->fd(), wt_buffer_.r_pos(), 
                        wt_buffer_.readable_bytes());
    if (n > 0) {
      wt_buffer_.Retrieve(n);
      if (wt_buffer_.readable_bytes() == 0) {
        conn_channel_->DisableWriting();
        if (conn_state_ == kDisconnecting) {
          ShutdownInLoop();
        }
      }
    } else {
      LOG_SYSERR << "TCPConnection::HandleWrite";
    }
  } else {
    LOG_TRACE << "Connection fd = " << conn_channel_->fd()
              << " is down, no more writing.";
  }
}

// Close Connection.
// Called in channel->HandleEvent(). Because life of channel is kept by
//   its owner class. We cannot destroy its owner in this callback.
// The destroy step should be delayed to pending task.
void TCPConnection::HandleClose() {
  loop_->AssertInLoopThread();
  LOG_TRACE << "fd = " << conn_channel_->fd() << " state = " << conn_state_;

  assert(conn_state_ == kConnected || conn_state_ == kDisconnecting);
  set_state(kDisconnected);
  conn_channel_->DisableAll();

  TCPConnectionPtr life_guard(shared_from_this());  // * use_count: 1 --> 2
  conn_cb_(life_guard);
  close_cb_(life_guard);
}

void TCPConnection::HandleError() {
  int sock_err_info;
  socklen_t optlen = static_cast<socklen_t>(sizeof(sock_err_info));

  if (::getsockopt(conn_channel_->fd(), SOL_SOCKET, SO_ERROR, &sock_err_info, 
                   &optlen) < 0) {
    LOG_SYSERR << "TCPConnection::HandleError";
  } else {
    LOG_ERROR << "TCPConnection::HandleError " << name_  << " SO_ERROR = "
              << sock_err_info;
  }
}

} // namespace net
} // namespace cppnetlib
