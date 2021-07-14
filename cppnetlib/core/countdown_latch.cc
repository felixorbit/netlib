#include "cppnetlib/core/countdown_latch.h"

namespace cppnetlib {

void CountdownLatch::Wait() {
  MutexLockGuard lock(mutex_);
  while (count_ > 0) {
    condition_.Wait();
  }
}

void CountdownLatch::CountDown() {
  MutexLockGuard lock(mutex_);
  --count_;
  if (count_ == 0) {
    condition_.NotifyAll();
  }
}

int CountdownLatch::get_count() const {
  MutexLockGuard lock(mutex_);
  return count_;
}

} // namespace cppnetlib