#include "cppnetlib/core/log_file.h"

#include <unistd.h>

namespace cppnetlib {

// public

LogFile::LogFile(const std::string& basename, off_t rollsize, bool locked, 
                 int flushgap, int checkfreq)
    : basename_(basename), rollsize_(rollsize), flushgap_(flushgap),
      checkfreq_(checkfreq), append_count_(0), start_period_(0), last_roll_(0),
      last_flush_(0), mutex_(locked ? new Mutex : nullptr) {
  assert(basename.find('/') == std::string::npos);
  RollFile();
}

LogFile::~LogFile() = default;

void LogFile::Append(const char* str_line, int len) {
  if (mutex_) {
    MutexLockGuard lock(*mutex_);
    UnlockedAppend(str_line, len);
  } else {
    UnlockedAppend(str_line, len);
  }
}

void LogFile::Flush() {
  if (mutex_) {
    MutexLockGuard lock(*mutex_);
    filler_ -> Flush();
  } else {
    filler_ -> Flush();
  }
}

// Resets filler_ with a pointer of new created FileFiller
bool LogFile::RollFile() {
  time_t now = 0;
  std::string filename = GetLogFileName(basename_, &now);
  time_t start = now / kRollPeriodsSeconds * kRollPeriodsSeconds;

  if (now > last_roll_) {
    last_roll_ = now;
    last_flush_ = now;
    start_period_ = start;

    filler_.reset(new FileFiller(filename));
    return true;
  }
  return false;
}

// private

// Formatted log file name likes:
//  {basename}.YYYYmmdd-HHMMSS.{pid}.log
std::string LogFile::GetLogFileName(const std::string& basename, time_t* now) {
  std::string filename;
  filename.reserve(basename.size() + 64);
  filename = basename;

  char timebuf[32];
  *now = time(nullptr);  // time(): gets seconds since epoch
  struct tm tm_obj;
  gmtime_r(now, &tm_obj);// gmtime(): transfers seconds to a broken-down time
  strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S", &tm_obj);
  filename += timebuf;   // strftime(): format time string

  char pidbuf[32];
  snprintf(pidbuf, sizeof(pidbuf), ".%d", getpid());
  filename += pidbuf;

  filename += ".log";
  return filename;
}

// 1. Appends a string to buffer
// 2. Checks Rolling condition(1): written size
// 3. When appending operations reach a certain number:
//    a) check Rolling condition(2): rolling periods
//    b) check Flushing condition: flush gap
void LogFile::UnlockedAppend(const char* str_line, int len) {
  filler_ -> Append(str_line, len);

  if (filler_ -> written_bytes() > rollsize_) {
    RollFile();
  } else {
    ++append_count_;
    if (append_count_ > checkfreq_) {
      append_count_ = 0;
      time_t now = time(nullptr);
      time_t this_period = now / kRollPeriodsSeconds * kRollPeriodsSeconds;
      if (this_period != start_period_) {
        RollFile();
      } else if (now - last_flush_ > flushgap_) {
        last_flush_ = now;
        filler_ -> Flush();
      }
    }
  }
}

} // namespace cppnetlib