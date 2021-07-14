#ifndef CPPNETLIB_CORE_COMMON_STRING_H_
#define CPPNETLIB_CORE_COMMON_STRING_H_

#include <string>

namespace cppnetlib {

class CommonString {
 public:
  CommonString(const char* str) : str_(str) {}
  CommonString(const std::string& str) : str_(str.c_str()) {}
  CommonString(const std::string&& str) : str_(str.c_str()) {}

  const char* c_str() const {
    return str_;
  }

 private:
  const char* str_;
};

} // namespace cppnetlib

#endif // CPPNETLIB_CORE_COMMON_STRING_H_