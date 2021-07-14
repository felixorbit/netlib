#ifndef CPPNETLIB_NET_TIMER_H_
#define CPPNETLIB_NET_TIMER_H_

#include <atomic>
#include <functional>

#include "cppnetlib/core/noncopyable.h"
#include "cppnetlib/core/time_anchor.h"

namespace cppnetlib {
namespace net {
  
class Timer : Noncopyable {
 public:
  Timer(std::function<void()> callback, TimeAnchor expiration, double interval)
      : callback_(callback), expiration_(expiration), interval_(interval),
        seq_id_(++num_created_), timer_pos_(-1) {}

  void Run() const { callback_(); }
  bool InHeap() const { return timer_pos_ >= 0; }

  // get value functions
  TimeAnchor expiration() const { return expiration_; }
  int64_t seq_id() const { return seq_id_; }
  int timer_pos() const { return timer_pos_; }
  static int64_t num_created() { return num_created_.load(); }
  // set value functions
  void set_timer_pos(size_t timer_pos);
  void set_timer_pos(const int timer_pos);

 private:
  static std::atomic_int64_t num_created_;

  const std::function<void()> callback_;
  TimeAnchor expiration_;
  const double interval_;
  const int64_t seq_id_;
  int timer_pos_;
};

} // namespace net
} // namespace cppnetlib

#endif // CPPNETLIB_NET_TIMER_H_
