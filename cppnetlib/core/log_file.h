#ifndef CPPNETLIB_CORE_LOG_FILE_H_
#define CPPNETLIB_CORE_LOG_FILE_H_

#include <memory>
#include "cppnetlib/core/noncopyable.h"
#include "cppnetlib/core/file_filler.h"
#include "cppnetlib/core/mutex.h"

namespace cppnetlib {

// Utilizes FileFiller to write log file.
// Record states and regularly check whether to rolling or flushing.
class LogFile : Noncopyable {
 public:
  LogFile(const std::string& basename, off_t rollsize, bool locked=true, 
          int flushgap = 3, int checkfreq = 1024);
  ~LogFile();
  
  // Appends a string to buffer. Checks whether to rolling or flush.
  void Append(const char* str_line, int len);
  // Forces to flush buffer
  void Flush();
  // Creates a new FileFiller for a new log file
  bool RollFile();

 private:
  void UnlockedAppend(const char*, int);
  // Returns a formated log file name.
  static std::string GetLogFileName(const std::string& basename, time_t* now);
  
  // Action Contion
  const std::string basename_;
  const off_t rollsize_;  // file size threshold(bytes) of log rolling
  const int flushgap_;    // buffer flush threshold
  const int checkfreq_;   // frequency of checking rolling and flushing
  
  // Logging State
  int append_count_;      // count append operations
  time_t start_period_;   // last log rolling period (such as day)
  time_t last_roll_;      // last log rolling time (in seconds)
  time_t last_flush_;     // last buffer flushing time (in seconds)

  std::unique_ptr<Mutex> mutex_;
  std::unique_ptr<FileFiller> filler_;

  const static int kRollPeriodsSeconds = 3600*24;  // defines rolling period 
};

} // namespace cppnetlib

#endif // CPPNETLIB_CORE_LOG_FILE_H_