#ifndef CPPNETLIB_HTTP_HTTP_REQUEST_H_
#define CPPNETLIB_HTTP_HTTP_REQUEST_H_

#include <cassert>
#include <map>

namespace cppnetlib {
namespace net {

class HTTPRequest {
 public:
  enum Method { kINVALID, kGET, kPOST, kHEAD, kPUT, kDELETE };
  enum Version { kUNKOWN, kHTTP10, kHTTP11 };

  HTTPRequest() : method_(kINVALID), version_(kUNKOWN) {}

  Method method() const { return method_; }
  Version version() const { return version_; }
  const std::string method_string() const {
    std::string method = "UNKOWN";
    switch (method_) {
      case kGET:
        method = "GET";
        break;
      case kPOST:
        method = "POST";
        break;
      case kHEAD:
        method = "HEAD";
        break;
      case kPUT:
        method = "PUT";
        break;
      case kDELETE:
        method = "DELETE";
        break;
      default:
        break;
    }
    return method;
  }
  const std::string& path() const { return path_; }
  const std::string& query() const { return query_; }
  const std::map<std::string, std::string> headers() const { return headers_; }
  std::string get_header(const std::string& field) const {
    std::string result;
    std::map<std::string, std::string>::const_iterator it = headers_.find(field);
    if (it != headers_.end()) {
      result = it->second;
    }
    return result;
  }

  bool set_method(std::string method) {
    assert(method_ == kINVALID);
    if (method == "GET") {
      method_ = kGET;
    } else if (method == "POST") {
      method_ = kPOST;
    } else if (method == "HEAD") {
      method_ = kHEAD;
    } else if (method == "PUT") {
      method_ = kPUT;
    } else if (method == "DELETE") {
      method_ = kDELETE;
    } else {
      method_ = kINVALID;
    }
    return method_ != kINVALID;
  }
  void set_version(Version version) { version_ = version; }
  void set_path(const std::string& path) { path_ = path; } 
  void set_query(const std::string& query) { query_ = query; }
  void add_header(const std::string& field, const std::string& value) {
    headers_[field] = value;
  }

  void swap(HTTPRequest& that) {
    std::swap(method_, that.method_);
    std::swap(version_, that.version_);
    path_.swap(that.path_);
    query_.swap(that.query_);
    headers_.swap(that.headers_);
  }

 private:
  Method method_;
  Version version_;
  std::string path_;
  std::string query_;
  std::map<std::string, std::string> headers_;
};

} // namespace net 
} // namespace cppnetlib

#endif // CPPNETLIB_HTTP_HTTP_REQUEST_H_