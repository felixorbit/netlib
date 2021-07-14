#include <unistd.h>
#include <cassert>
#include <cstdio>

#include "cppnetlib/net/event_loop.h"
#include "cppnetlib/core/thread.h"
#include "cppnetlib/net/timer_id.h"

using namespace cppnetlib;
using namespace cppnetlib::net;

EventLoop* g_loop;

void Callback() {
  printf("callback(): pid = %d, tid = %d\n", getpid(), thisthread::tid());

  // Negative test: the creation of a new EventLoop obj will be aborted.
  // Since in this thread, we have an EventLoop obj already.
  EventLoop another_loop;
}

void ThreadFunc() {
  printf("threadFunc(): pid = %d, tid = %d\n", getpid(), thisthread::tid());

  assert(EventLoop::GetCurrentThreadLoop() == nullptr);
  EventLoop loop;
  assert(EventLoop::GetCurrentThreadLoop() == &loop);

  loop.RunAfter(2.0, Callback);
  loop.Loop();
}

int main() {
  printf("main(): pid = %d, tid = %d\n", getpid(), thisthread::tid());

  assert(EventLoop::GetCurrentThreadLoop() == nullptr);
  EventLoop loop;
  assert(EventLoop::GetCurrentThreadLoop() == &loop);

  Thread thread(ThreadFunc);
  thread.Start();

  loop.Loop();
}