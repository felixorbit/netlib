#ifndef CPPNETLIB_CORE_COUNTDOWN_LATCH_H_
#define CPPNETLIB_CORE_COUNTDOWN_LATCH_H_

#include "cppnetlib/core/noncopyable.h"
#include "cppnetlib/core/mutex.h"
#include "cppnetlib/core/condition.h"


namespace cppnetlib {

// Advanced Component for Concurrent Programming
// 1. CountdownLatch
class CountdownLatch : Noncopyable {
 public:
  explicit CountdownLatch(int count) 
      : mutex_(), condition_(mutex_), count_(count) {}
  
  void Wait();
  void CountDown();

  int get_count() const;

 private:
  mutable Mutex mutex_;  // can be modified by in const member functions
  Condition condition_;
  int count_;
};

} // namespace cppnetlib

#endif // CPPNETLIB_CORE_COUNTDOWN_LATCH_H_