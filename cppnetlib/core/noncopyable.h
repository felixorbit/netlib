#ifndef CPPNETLIB_CORE_NONCOPYABLE_H_
#define CPPNETLIB_CORE_NONCOPYABLE_H_
namespace cppnetlib {

class Noncopyable {
 public:
  Noncopyable(const Noncopyable&) = delete;
  void operator=(const Noncopyable&) = delete;

 protected:
  Noncopyable() = default;
  ~Noncopyable() = default;
};

} // namespace cppnetlib

#endif // CPPNETLIB_CORE_NONCOPYABLE_H_