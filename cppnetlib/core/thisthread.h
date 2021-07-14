#ifndef CPPNETLIB_CORE_THIS_THREAD_H_
#define CPPNETLIB_CORE_THIS_THREAD_H_

namespace cppnetlib {
  
namespace thisthread {

extern __thread int         t_cached_tid;  // Thread Local Storage (GCC build-in)
extern __thread char        t_tid_str[32];
extern __thread int         t_tid_strlen;
extern __thread const char* t_thread_name;

void CacheTid();

// Returns tid, and caches tid on the first call
inline int tid() {
  if (__builtin_expect(t_cached_tid == 0, 0))  // expect "t_cached_tid == 0" is false
    CacheTid();                                // for optimizing branch prediction. (GCC build-in)
  return t_cached_tid;
}

inline const char* tid_str() {
  return t_tid_str;
}

inline const char* thread_name() {
  return t_thread_name;
}

inline int tid_str_len() {
  return t_tid_strlen;
}


} // namespace thisthread
} // namespace cppnetlib


#endif // CPPNETLIB_CORE_THIS_THREAD_H_