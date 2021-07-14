#ifndef CPPNETLIB_CORE_MUTEX_H_
#define CPPNETLIB_CORE_MUTEX_H_

#include <pthread.h>

#include <cassert>

#include "cppnetlib/core/noncopyable.h"
#include "cppnetlib/core/thisthread.h"

namespace cppnetlib {

// Resource: A wrapper of POSIX mutex
class Mutex : Noncopyable {
 public:
  Mutex() : holder_(0) { pthread_mutex_init(&mutex_, nullptr); }
  ~Mutex() {
    assert(holder_ == 0);
    pthread_mutex_destroy(&mutex_);
  }

  bool IsLockedByThisThread() const { return holder_ == thisthread::tid(); }
  void LockedAssert() const { assert(IsLockedByThisThread()); }
  
  // Records lock holder in addition
  void Lock() {  
    pthread_mutex_lock(&mutex_);  // the order is important
    set_holder();
  }
  void Unlock() {
    unset_holder();               // opposite order
    pthread_mutex_unlock(&mutex_);
  }

  pthread_mutex_t* get_pthread_mutex() { return &mutex_; }
  
 private:
  void set_holder() { holder_ = thisthread::tid(); }
  void unset_holder() { holder_ = 0; }

  // For Condition Variable's special needs:
  //  1. direct access of pthread_mutex
  //  2. manually unset holder when pthread_cond_wait() implicit unlocks 
  //     pthread_mutex and set holder again when it returns
  friend class Condition;
  // Additional holder manager in RAII style
  class HolderGuard : Noncopyable {
   public:
    explicit HolderGuard(Mutex& owner) : mutex_owner_(owner) {
      mutex_owner_.unset_holder();
    }
    ~HolderGuard() { mutex_owner_.set_holder(); }
   private:
    Mutex& mutex_owner_;
  };

  pthread_mutex_t mutex_;
  pid_t holder_;
};


// Usage of Resource: RAII method
class MutexLockGuard : Noncopyable {
 public:
  explicit MutexLockGuard(Mutex& mutex) : mutex_(mutex) { mutex_.Lock(); }
  ~MutexLockGuard() { mutex_.Unlock(); }

 private:
  Mutex& mutex_;
};

} // namespace cppnetlib

#endif // CPPNETLIB_CORE_MUTEX_H_