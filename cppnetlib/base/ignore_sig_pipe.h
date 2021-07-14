#ifndef CPPNETLIB_BASE_IGNORE_SIG_PIPE_H_
#define CPPNETLIB_BASE_IGNORE_SIG_PIPE_H_

#include <signal.h>

namespace cppnetlib {
namespace net {
namespace detail {

class IgnoreSigPipe {
 public:
  IgnoreSigPipe() {
    ::signal(SIGPIPE, SIG_IGN);
  }
};

} // namespace detail
} // namespace net
} // namespace cppnetlib


#endif