#ifndef CPPNETLIB_CORE_CONDITION_H_
#define CPPNETLIB_CORE_CONDITION_H_

#include "cppnetlib/core/noncopyable.h"
#include "cppnetlib/core/mutex.h"

namespace cppnetlib {

// Resourse: A wrapper of POSIX condition variable
class Condition : Noncopyable {
 public:
  explicit Condition(Mutex& mutex) : mutex_(mutex) {
    pthread_cond_init(&cond_, nullptr);
  }
  ~Condition() {
    pthread_cond_destroy(&cond_);
  }

  // To keep behavior consistent with pthread_cond_wait(),
  //   HolderGuard manages mutex's holer in RAII style.
  void Wait() {
    Mutex::HolderGuard hg(mutex_);
    pthread_cond_wait(&cond_, mutex_.get_pthread_mutex());
  }
  // Returns true if time out.
  bool WaitSecs(double secs);

  void Notify() { pthread_cond_signal(&cond_); }
  void NotifyAll() { pthread_cond_broadcast(&cond_); }

 private:
  Mutex& mutex_;
  pthread_cond_t cond_;
};

} // namespace cppnetlib

#endif // CPPNETLIB_CORE_CONDITION_H_