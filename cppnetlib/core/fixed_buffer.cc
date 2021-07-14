#include "cppnetlib/core/fixed_buffer.h"

namespace cppnetlib {
namespace detail {

template class FixedBuffer<kSmallBuffer>;  // explicit instantiation of a template class (definition)
template class FixedBuffer<kLargeBuffer>;

template<int SIZE>
const char* FixedBuffer<SIZE>::DebugString() {
  *cur_ = '\0';
  return data_;
}

template<int SIZE>
void FixedBuffer<SIZE>::CookieStart() {}

template<int SIZE>
void FixedBuffer<SIZE>::CookieEnd() {}

} // namespace detail
} // namespace cppnetlib