#include "thread_pool.h"

namespace cppnetlib {

// public
ThreadPool::ThreadPool(const std::string& name)
    : running_(false), name_(name), max_queue_size_(0), mutex_(),
      cond_not_empty_(mutex_), cond_not_full_(mutex_) {}

ThreadPool::~ThreadPool() {
  if (running_)
    Stop();
}

void ThreadPool::Start(int num_thread) {
  if (num_thread == 0)
    return;
  assert(threads_.empty());
  running_ = true;
  threads_.reserve(num_thread);

  for (int i = 0; i != num_thread; ++i) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", i+1);
    threads_.emplace_back(new Thread(std::bind(&ThreadPool::RunTask, this),
                                     name_ + buf));
    threads_[i] -> Start();
  }
}

void ThreadPool::Stop() {
  {
    MutexLockGuard lock(mutex_);
    running_ = false;
    cond_not_empty_.NotifyAll();
    cond_not_full_.NotifyAll();
  }

  for (auto& thread : threads_)
    thread -> Join();
}

void ThreadPool::AddTask(Task task) {
  MutexLockGuard lock(mutex_);
  while (is_full())
    cond_not_full_.Wait();
  if (!running_)
    return;
  assert(!is_full());

  task_queue_.push_back(std::move(task));
  cond_not_empty_.NotifyAll();
}

size_t ThreadPool::num_tasks() const {
  MutexLockGuard lock(mutex_);
  return task_queue_.size();
}


// private
void ThreadPool::RunTask() {
  if (init_callback_)
    init_callback_();
  while (running_) {
    Task task(TakeTask());
    if (task)
      task();
  }
}

ThreadPool::Task ThreadPool::TakeTask() {
  MutexLockGuard lock(mutex_);
  while (task_queue_.empty() && running_)
    cond_not_empty_.Wait();       // wait for task to arrive
  
  Task task;
  if (!task_queue_.empty()) {
    task = task_queue_.front();
    task_queue_.pop_front();
    if (max_queue_size_ > 0)
      cond_not_full_.NotifyAll(); // take a task and notify all
  }
  return task;
}

bool ThreadPool::is_full() const {
  mutex_.LockedAssert();
  return max_queue_size_ > 0 && task_queue_.size() >= max_queue_size_;
}

} // namespace cppnetlib