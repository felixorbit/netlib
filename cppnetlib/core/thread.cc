#include "cppnetlib/core/thread.h"

#include <sys/prctl.h>

namespace cppnetlib {

namespace thread_impl {

struct ThreadData {
  using ThreadFunc = Thread::ThreadFunc;
  ThreadFunc func_;
  std::string name_;
  pid_t* tid_ptr_;
  CountdownLatch* latch_ptr_;

  ThreadData(ThreadFunc func, const std::string& name, pid_t* tid_ptr, 
             CountdownLatch* latch_ptr)
      : func_(std::move(func)), 
        name_(name), 
        tid_ptr_(tid_ptr), 
        latch_ptr_(latch_ptr) {}

  ThreadData(ThreadFunc&& func, const std::string& name, pid_t* tid_ptr, 
             CountdownLatch* latch_ptr)
      : func_(func), 
        name_(name), 
        tid_ptr_(tid_ptr), 
        latch_ptr_(latch_ptr) {}
  
  void RunInThread() {
    *tid_ptr_ = thisthread::tid();
    tid_ptr_ = nullptr;

    latch_ptr_ -> CountDown();
    latch_ptr_ = nullptr;

    thisthread::t_thread_name = name_.c_str();
    prctl(PR_SET_NAME, thisthread::t_thread_name);

    func_();
    thisthread::t_thread_name = "finished";
  }
};

void* StartThread(void* args) {
  ThreadData* data = static_cast<ThreadData*>(args);
  data -> RunInThread();
  delete data;
  return nullptr;
}

} // namespace thread_impl

// static member must be defined outside class
std::atomic_int32_t Thread::num_created_(0);

Thread::Thread(ThreadFunc func, const std::string& name) 
    : started_(false), joined_(false), pthread_id_(0), tid_(0), 
      func_(std::move(func)), name_(name), latch_(1) {
  SetDefaultName();
} 

// When Thread object lives shorter than thread,
//   make thread detached to avoid resource leakage.
Thread::~Thread() {
  if (started_ && !joined_)
    pthread_detach(pthread_id_);
}

void Thread::Start() {
  assert(!started_);
  started_ = true;
  thread_impl::ThreadData* data = new thread_impl::ThreadData(func_, name_, 
                                                              &tid_, &latch_);
  // returns 0 if success
  if (!pthread_create(&pthread_id_, NULL, &thread_impl::StartThread, data)) {
    latch_.Wait();
    assert(tid_ > 0);
  } else {
    started_ = false;
    delete data;
  }
}

int Thread::Join() {
  assert(started_);
  assert(!joined_);
  joined_ = true;
  return pthread_join(pthread_id_, NULL);
}

void Thread::SetDefaultName() {
  int num = IncDefaultNum();
  if (name_.empty()) {
    char buf[32];
    snprintf(buf, sizeof(buf), "Thread%d", num);
    name_ = buf;
  }
}

} // namespace cppnetlib