#include <unistd.h>
#include <cstdio>

#include "cppnetlib/core/thread_pool.h"
#include "cppnetlib/core/countdown_latch.h"
#include "cppnetlib/core/thisthread.h"
#include "cppnetlib/core/logger.h"

using namespace cppnetlib;

void printTid() {
  printf("tid=%d\n", thisthread::tid());
}

void printString(const std::string& str) {
  LOG_INFO << str;
  usleep(100*1000);
}

void test(int max_qsize) {
  LOG_WARN << "Test ThreadPool with max task queue size: " << max_qsize;
  
  ThreadPool pool("Main Thread Pool");
  pool.set_max_queue_size(max_qsize);
  pool.Start(5);

  LOG_WARN << "Adding Tasks";
  pool.AddTask(printTid);
  pool.AddTask(printTid);
  for (int i = 0; i < 100; i++) {
    char buf[32];
    snprintf(buf, sizeof(buf), "task %d", i);
    pool.AddTask(std::bind(printString, std::string(buf)));
  }
  LOG_WARN << "Done";

  // wait all tasks done
  CountdownLatch latch(1);
  pool.AddTask(std::bind(&CountdownLatch::CountDown, &latch));
  latch.Wait();
  pool.Stop();
}

int main() {
  test(1);
  test(5);
  test(10);
  test(50);
}