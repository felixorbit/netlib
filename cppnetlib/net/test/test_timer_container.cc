#include <unistd.h>
#include <cstdio>

#include "cppnetlib/net/event_loop.h"
#include "cppnetlib/net/io_thread.h"
#include "cppnetlib/net/timer_id.h"

using namespace cppnetlib;
using namespace cppnetlib::net;

// global counter
int cnt = 0;

// global accessible event loop
EventLoop* g_loop;

void ShowTid() {
  printf("pid = %d, tid = %d\n", getpid(), thisthread::tid());
  printf("now %s\n", TimeAnchor::Now().ToStr().c_str());
}

void Print(const char* msg) {
  printf("msg %s %s\n", TimeAnchor::Now().ToStr().c_str(), msg);
  if (++cnt == 5) {
      g_loop->Quit();
  }
}

void CancelTimer(TimerID timerid) {
  g_loop->CancelTimer(timerid);
  printf("cancel at %s\n", TimeAnchor::Now().ToStr().c_str());
}

int main() {
  ShowTid();
  sleep(1);

  // test single thread eventloop
  {
    EventLoop loop;
    g_loop = &loop;
    Print("main loop");

    // register timeout callbacks
    loop.RunAfter(1.0, std::bind(Print, "once1"));
    loop.RunAfter(1.5, std::bind(Print, "once1.5"));
    loop.RunAfter(2.5, std::bind(Print, "once2.5"));
    loop.RunAfter(3.5, std::bind(Print, "once3.5"));
    TimerID t1 = loop.RunAfter(4.5, std::bind(Print, "once4.5"));
    loop.RunAfter(4.2, std::bind(CancelTimer, t1));
    loop.RunAfter(4.8, std::bind(CancelTimer, t1));

    loop.Loop();
    Print("main loop exits");
  }

  sleep(1);
  // test timer container with IO thread
  {
    IOThread loop_thread;
    EventLoop* loop = loop_thread.Start();
    loop->RunAfter(2.0, ShowTid);
    sleep(3);
    Print("thread loop exits");
  }
}