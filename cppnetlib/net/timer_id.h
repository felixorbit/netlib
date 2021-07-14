#ifndef CPPNETLIB_NET_TIMER_ID_H_
#define CPPNETLIB_NET_TIMER_ID_H_

namespace cppnetlib {
namespace net {

class Timer;

class TimerID {
 public:
  TimerID() : timer_(nullptr), seq_id_(0) {}
  TimerID(Timer* timer, int64_t seq_id) : timer_(timer), seq_id_(seq_id) {}
  ~TimerID() = default;

  friend class TimerContainer;

 private:
  Timer* timer_;
  int64_t seq_id_;
};

} // namespace net
} // namespace cppnetlib

#endif // CPPNETLIB_NET_TIMER_ID_H_