#include "cppnetlib/core/thisthread.h"

#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <type_traits>
#include <cassert>
#include <cstdio>

namespace cppnetlib {
namespace thisthread {

__thread int         t_cached_tid = 0;
__thread char        t_tid_str[32];
__thread int         t_tid_strlen = 6;
__thread const char* t_thread_name = "unkown";

static_assert(std::is_same<int, pid_t>::value, "pid_t should apper as an int!");

pid_t gettid() {
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

void CacheTid() {
  assert(t_cached_tid == 0);
  t_cached_tid = gettid();
  t_tid_strlen = snprintf(t_tid_str, sizeof(t_tid_str), "%5d ", t_cached_tid);
}


} // namespace thisthread
} // namespace cppnetlib