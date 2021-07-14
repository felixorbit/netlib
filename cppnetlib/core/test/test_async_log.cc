#include <sys/resource.h>
#include <unistd.h>
#include <cstdio>

#include "cppnetlib/core/async_log.h"
#include "cppnetlib/core/logger.h"
#include "cppnetlib/core/time_anchor.h"

using namespace cppnetlib;

off_t kRollSize = 1 * 1000 * 1000;

AsyncLog* g_asynclog = nullptr;


void AsyncOutput(const char* msg, int len) {
    g_asynclog->Append(msg, len);
}

void Bench(bool long_log) {
  const int kBatch = 1000;

  std::string empty = " ";
  std::string long_str(3000, 'X');
  long_str += " ";

  int cnt = 0;
  for (int t = 0; t < 30; t++) {
    TimeAnchor start = TimeAnchor::Now();
    for (int i = 0; i < kBatch; i++) {
      LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
               << (long_log ? long_str : empty) << cnt;
      cnt++;
    }
    TimeAnchor end = TimeAnchor::Now();

    printf("%f\n", TimeDiff(end, start)*1000000/kBatch);
    struct timespec ts = { 0, 500*1000*1000 };
    nanosleep(&ts, nullptr);
  }
}

int main(int argc, char* argv[]) {
  {
    size_t kOneGB = 1000*1024*1024;
    rlimit rl = { 2*kOneGB, 2*kOneGB };
    // set address space limit
    setrlimit(RLIMIT_AS, &rl);
  }

  printf("pid = %d\n",getpid());

  Logger::set_output(AsyncOutput);

  char name[256] = { '\0' };
  strncpy(name, argv[0], sizeof(name) - 1);

  AsyncLog log(::basename(name), kRollSize);
  log.Start();
  g_asynclog = &log;

  bool long_log = argc > 1;
  Bench(long_log);
}