#include "cppnetlib/core/time_anchor.h"

#include <cinttypes>  // definition of PRId64
#include <sys/time.h>

namespace cppnetlib {

std::string TimeAnchor::ToStr() const {
  char buf[32];
  int64_t seconds = stamp_ / kMSecsPerSec;
  int64_t residual_micro = stamp_ % kMSecsPerSec;
  // For int64_t (cross 32/64bit platform)
  snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds, residual_micro);
  return buf;
}

std::string TimeAnchor::FormatMicro() const {
  char buf[64];
  time_t seconds = static_cast<time_t>(stamp_ / kMSecsPerSec);
  int residual_micro = static_cast<int>(stamp_ % kMSecsPerSec);
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);
  snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d",
           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, residual_micro);
  return buf;
}

std::string TimeAnchor::FormatSeconds() const {
  char buf[64];
  time_t seconds = static_cast<time_t>(stamp_ / kMSecsPerSec);
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);
  snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d",
           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
  return buf;
}

TimeAnchor TimeAnchor::Now() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t seconds = tv.tv_sec;
  return TimeAnchor(seconds * kMSecsPerSec + tv.tv_usec);
}


} // namespace cppnetlib