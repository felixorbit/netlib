#include <vector>
#include <cstdio>
#include <memory>

#include "cppnetlib/core/mutex.h"
#include "cppnetlib/core/thread.h"
#include "cppnetlib/core/time_anchor.h"

using namespace cppnetlib;
using namespace std;

Mutex g_mutex;
vector<int> g_vec;
const int kCount = 10000000;

// every thread push kCount values into g_vec
void threadFunc() {
  MutexLockGuard lock(g_mutex);
  for (int i = 0; i < kCount; i++) {
    g_vec.push_back(i);
  }
}

int main() {
  const int kMaxThreads = 8;
  g_vec.reserve(kCount * kMaxThreads);
  for (int nthreads = 1; nthreads <= kMaxThreads; nthreads++) {
    vector<unique_ptr<Thread>> threads;
    g_vec.clear();

    auto start = TimeAnchor::Now();
    // create thread objs and start every thread
    for (int i = 0; i < nthreads; i++) {
      threads.emplace_back(new Thread(threadFunc));
      threads.back()->Start();
    }
    for (int i = 0; i < nthreads; i++) {
      threads[i]->Join();
    }
    printf("%d thread(s) with lock, time cost: %f\n", nthreads, 
           TimeDiff(TimeAnchor::Now(), start));

    // check order
    for (int i = 0; i < nthreads; i++) {
      for (int j = i*kCount, k = 0; k < kCount; k++, j++) {
        assert(k == g_vec[j]);
      }
    }
  }
  printf("Totally %d threads created.\n", Thread::num_created());
}