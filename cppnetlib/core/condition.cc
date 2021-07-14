#include "cppnetlib/core/condition.h"

#include <errno.h>
#include <cstdint>

namespace cppnetlib {

bool Condition::WaitSecs(double secs) {
  struct timespec abstime;

  clock_gettime(CLOCK_REALTIME, &abstime);

  const int64_t nanoweight = 1000000000;
  int64_t nanosecs = static_cast<int64_t>(secs * nanoweight);
  
  abstime.tv_sec += static_cast<time_t>(
      (abstime.tv_nsec + nanosecs) / nanoweight);
  abstime.tv_nsec = static_cast<long>(
      (abstime.tv_nsec + nanosecs) % nanoweight);
  Mutex::HolderGuard hg(mutex_);
  return ETIMEDOUT == pthread_cond_timedwait(&cond_, 
                                             mutex_.get_pthread_mutex(), 
                                             &abstime);
}

} // namespace cppnetlib