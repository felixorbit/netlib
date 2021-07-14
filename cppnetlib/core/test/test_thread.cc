#include <unistd.h>
#include <cstdio>
#include <string>

#include "cppnetlib/core/thread.h"
#include "cppnetlib/core/thisthread.h"

using namespace cppnetlib;


void threadFunc() {
  printf("thread tid=%d\n", thisthread::tid());
}

void threadFunc1(int x) {
  printf("thread tid=%d, x=%d\n", thisthread::tid(), x);
}

void sleepsecs(int seconds) {
  timespec t = {seconds, 0};
  nanosleep(&t, nullptr);
}

int main() {
  printf("process pid=%d, thread tid=%d\n", ::getpid(), thisthread::tid());

  Thread t1(threadFunc);
  t1.Start();
  printf("t1's tid=%d\n", t1.tid());
  t1.Join();

  Thread t2(std::bind(threadFunc1, 128), "thread with a param");
  t2.Start();
  printf("t2's tid=%d\n", t2.tid());
  t2.Join();

  {
    Thread t3(threadFunc);
    t3.Start();
  } // t3 obj may be destroyed before thread func finished

  sleepsecs(2);

  {
    Thread t4(threadFunc);
    t4.Start();
    sleepsecs(2);
  } // t4 destruct after thread func

  printf("number of created threads %d\n", Thread::num_created());
}