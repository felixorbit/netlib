#ifndef CPPNETLIB_CORE_TIME_ANCHOR_H_
#define CPPNETLIB_CORE_TIME_ANCHOR_H_

#include <string>

namespace cppnetlib {

// Keeps time stamp as microseconds
// Returns different string-style time representations.
class TimeAnchor {
 public:
  TimeAnchor() : stamp_(0) {}
  TimeAnchor(int64_t stamp) : stamp_(stamp) {}

  int64_t time_micro() const { return stamp_; }
  int64_t time_seconds() const {
    return static_cast<time_t>(stamp_ / kMSecsPerSec);
  }

  // Returns sec.micsec
  std::string ToStr() const;
  // Returns YYYY-MM-DD hh:mm:ss.msssss
  std::string FormatMicro() const;
  // Returns YYYY-MM-DD hh:mm:ss
  std::string FormatSeconds() const;

  // Returns current time stamp
  static TimeAnchor Now();

  static const int kMSecsPerSec = 1000000;

 private:
  // Micro seconds since epoch
  int64_t stamp_;
};

inline bool operator<(TimeAnchor lhs, TimeAnchor rhs) {
  return lhs.time_micro() < rhs.time_micro();
}

inline bool operator==(TimeAnchor lhs, TimeAnchor rhs) {
  return lhs.time_micro() == rhs.time_micro();
}

inline bool operator<=(TimeAnchor lhs, TimeAnchor rhs) {
  return lhs == rhs || lhs < rhs;
}

inline TimeAnchor operator-(TimeAnchor lhs, TimeAnchor rhs) {
  return TimeAnchor(lhs.time_micro() - rhs.time_micro());
}

inline double TimeDiff(TimeAnchor high, TimeAnchor low) {
  TimeAnchor diff = high - low;
  return static_cast<double>(diff.time_micro()) / TimeAnchor::kMSecsPerSec;
}

} // namespace cppnetlib

#endif // CPPNETLIB_CORE_TIME_ANCHOR_H_