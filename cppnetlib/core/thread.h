#ifndef CPPNETLIB_CORE_THREAD_H_
#define CPPNETLIB_CORE_THREAD_H_

#include "cppnetlib/core/noncopyable.h"
#include "cppnetlib/core/countdown_latch.h"

#include <functional>
#include <atomic>

namespace cppnetlib {

// Maintains a thread in two ways:
//  1. Thread object lives longer than thread. Makes thread joined.(Join)
//  2. Thread object lives shorter than thread. Makes thread detached.(dtor)
class Thread : Noncopyable {
 public:
  using ThreadFunc = std::function<void ()>;
  explicit Thread(ThreadFunc func, const std::string& name = std::string());
  ~Thread();

  // Starts a Thread will:
  //  1. get and set pthread_id and tid
  //  2. countdown latch ???
  //  3. execute func
  void Start();
  int Join();

  bool started() const { return started_; }
  pid_t tid() const {return tid_; }
  const std::string& name() const { return name_; }
  static int num_created() { return num_created_.load(); }

 private:
  void SetDefaultName();
  int IncDefaultNum() { return ++num_created_; }

  bool started_;
  bool joined_;
  pthread_t pthread_id_;  // unique in process environment
  pid_t tid_;             // globally unique
  ThreadFunc func_;
  std::string name_;
  CountdownLatch latch_;  /* Why needs a CountDownLatch? */

  static std::atomic_int32_t num_created_;  // atomic data type
};


} // namespace thread

#endif // CPPNETLIB_CORE_THREAD_H_