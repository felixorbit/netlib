#include <cstdio>
#include <cinttypes>

#include "cppnetlib/core/log_stream.h"
#include "cppnetlib/core/time_anchor.h"

using namespace cppnetlib;

const size_t N = 1000000;

template<typename T>
void BenchStandardIO(const char* fmtstr) {
  char buf[32];
  TimeAnchor start(TimeAnchor::Now());
  for (size_t i = 0; i != N; ++i) {
    snprintf(buf, sizeof(buf), fmtstr, (T)(i));
  }
  TimeAnchor end(TimeAnchor::Now());
  printf("Printf cost time: %f\n", TimeDiff(end, start));
}

template<typename T>
void BenchLogStream() {
  TimeAnchor start(TimeAnchor::Now());
  LogStream os;
  for (size_t i = 0; i != N; ++i) {
    os << (T)(i);
    os.ResetBuffer();
  }
  TimeAnchor end(TimeAnchor::Now());
  printf("LogStream cost time: %f\n", TimeDiff(end, start));
}


int main() {
  puts("int");
  BenchStandardIO<int>("%d");
  BenchLogStream<int>();

  puts("double");
  BenchStandardIO<double>("%.12g");
  BenchLogStream<double>();

  puts("int64_t");
  BenchStandardIO<int64_t>("%" PRId64);
  BenchLogStream<int64_t>();

  puts("void*");
  BenchStandardIO<void*>("%p");
  BenchLogStream<void*>();
}