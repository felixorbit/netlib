#include "cppnetlib/net/timer.h"

namespace cppnetlib {
namespace net {

std::atomic_int64_t Timer::num_created_;

void Timer::set_timer_pos(size_t timer_pos) { 
  timer_pos_ = static_cast<int>(timer_pos);
}

void Timer::set_timer_pos(const int timer_pos) { 
  timer_pos_ = timer_pos; 
}

} // namespace net
} // namespace cppnetlib
