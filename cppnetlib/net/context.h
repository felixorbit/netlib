#ifndef CPPNETLIB_NET_CONTEXT_H_
#define CPPNETLIB_NET_CONTEXT_H_

namespace cppnetlib {
namespace net {

class Buffer;

class Context {
 public:
  virtual bool ParseContext(Buffer*) = 0;
  virtual ~Context() = default;
};

} // namespace net
} // namespace cppnetlib

#endif // CPPNETLIB_NET_CONTEXT_H_