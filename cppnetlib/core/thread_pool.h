#ifndef CPPNETLIB_THREAD_POOL_H_
#define CPPNETLIB_THREAD_POOL_H_

#include "cppnetlib/core/noncopyable.h"
#include "cppnetlib/core/thread.h"

#include <vector>
#include <deque>
#include <memory>

namespace cppnetlib {

class ThreadPool : Noncopyable {
 public:
  using Task = std::function<void ()>;

  explicit ThreadPool(const std::string& name = std::string("thread pool"));
  ~ThreadPool();

  void Start(int num_thread);
  void Stop();
  void AddTask(Task);

  void set_max_queue_size(int max_size) { max_queue_size_ = max_size; }
  void set_init_callback(const Task& callback) { init_callback_ = callback; }
  const std::string& name() const { return name_; }
  size_t num_tasks() const;

 private:
  bool is_full() const;
  void RunTask();
  Task TakeTask();

  bool running_;
  std::string name_;
  Task init_callback_;
  std::vector<std::unique_ptr<Thread>> threads_;
  std::deque<Task> task_queue_;
  size_t max_queue_size_;
  
  mutable Mutex mutex_;  // protect access of task_queue
  Condition cond_not_empty_;  // no task in queue
  Condition cond_not_full_;   // tasks in queue need to be processed
};

} // namespace cppnetlib

#endif // CPPNETLIB_THREAD_POOL_H_