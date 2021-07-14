#include "cppnetlib/http/http_server.h"
#include "cppnetlib/http/http_context.h"
#include "cppnetlib/http/http_request.h"
#include "cppnetlib/http/http_response.h"
#include "cppnetlib/core/logger.h"

namespace cppnetlib {
namespace net {

namespace detail {

void DefaultHTTPCallback(const HTTPRequest&, HTTPResponse* response) {
  response->set_status_code(HTTPResponse::kNotFound);
  response->set_status_message("Not Found");
  response->set_close_connection(true);
}

} // namespace detail

using std::placeholders::_1;
using std::placeholders::_2;

HTTPServer::HTTPServer(EventLoop* loop, const InetAddr& listen_addr, 
                       const std::string& name) 
    : server_(loop, listen_addr, name), 
      http_callback_(detail::DefaultHTTPCallback) {
  server_.set_connection_callback(std::bind(&HTTPServer::OnConnection, this, _1));
  server_.set_message_callback(std::bind(&HTTPServer::OnMessage, this, _1, _2));
}

// public

void HTTPServer::Start() {
  LOG_WARN << "HTTPServer[" << server_.name() << "] starts listening on "
           << server_.ip_port();
  server_.Start();
}


// private

// Bind a Context object to TCPConnection, because the parsing of HTTP request
//  is not completed once.
void HTTPServer::OnConnection(const TCPConnectionPtr& conn) {
  if (conn->connected()) {
    conn->set_context(new HTTPContext());
    LOG_TRACE << conn->sock_addr().GetIPPortStr() << " -> "
              << conn->peer_addr().GetIPPortStr() << " is "
              << (conn->connected() ? "connected" : "down");
  }
}

// Replace boost::any with dynamic_cast. Performance issues?
void HTTPServer::OnMessage(const TCPConnectionPtr& conn, Buffer* buf) {
  HTTPContext* context = dynamic_cast<HTTPContext*>(conn->mutable_context());
  if (!context->ParseContext(buf)) {
    conn->Send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->Shutdown();
  }

  if (context->GotAll()) {
    OnRequest(conn, context->request());
    context->Reset();
  }
}

void HTTPServer::OnRequest(const TCPConnectionPtr& conn, 
                           const HTTPRequest& request) {
  const std::string& connection = request.get_header("Connection");
  bool close = (connection == "close") || 
      (request.version() == HTTPRequest::kHTTP10 && connection != "Keep-Alive");
  HTTPResponse response(close);
  http_callback_(request, &response);
  
  Buffer buf;
  response.AppendToBuffer(&buf);
  conn->Send(&buf);
  if (response.close_connection()) {
    conn->Shutdown();
  }
}

} // namespace net
} // namespace cppnetlib
