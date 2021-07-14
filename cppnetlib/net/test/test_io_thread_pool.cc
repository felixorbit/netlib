#include <unistd.h>
#include <cstdio>

#include "cppnetlib/net/io_thread_pool.h"
#include "cppnetlib/net/event_loop.h"
#include "cppnetlib/core/thread.h"
#include "cppnetlib/net/timer_id.h"

using namespace cppnetlib;
using namespace cppnetlib::net;


void ShowId(EventLoop* loop = nullptr) {
  printf("showId: pid = %d, tid = %d, loop = %p\n", getpid(), thisthread::tid(), loop);
}

void ThreadFunc(EventLoop* loop) {
  printf("threadFunc: pid = %d, tid = %d, loop = %p\n", getpid(), thisthread::tid(), loop);
}

int main() {
  ShowId();

  EventLoop loop;
  loop.RunAfter(11, std::bind(&EventLoop::Quit, &loop));

  // only main thread
  {
    printf("Single thread %p:\n", &loop);
    IOThreadPool pool(&loop, "single");
    pool.set_thread_num(0);
    pool.Start(ThreadFunc);
    assert(pool.GetNextLoop() == &loop);
    assert(pool.GetNextLoop() == &loop);
    assert(pool.GetNextLoop() == &loop);
  }

  // main thread + another IOThread
  {
    printf("Another Thread:\n");
    IOThreadPool pool(&loop, "another");
    pool.set_thread_num(1);
    pool.Start(ThreadFunc);
    EventLoop* next_loop = pool.GetNextLoop();
    next_loop->RunAfter(2, std::bind(ShowId, next_loop));
    assert(next_loop != &loop);
    assert(next_loop == pool.GetNextLoop());
    assert(next_loop == pool.GetNextLoop());
    ::sleep(3);
  }

  // main thread + 3 other threads
  {
    printf("Three other threads:\n");
    IOThreadPool pool(&loop, "three");
    pool.set_thread_num(3);
    pool.Start(ThreadFunc);
    EventLoop* next_loop = pool.GetNextLoop();
    next_loop->RunInLoop(std::bind(ShowId, next_loop));
    assert(next_loop != &loop);
    assert(next_loop != pool.GetNextLoop());
    assert(next_loop != pool.GetNextLoop());
    assert(next_loop == pool.GetNextLoop());
  }

  loop.Loop();
}