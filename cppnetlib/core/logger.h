#ifndef CPPNETLIB_CORE_LOGGER_H_
#define CPPNETLIB_CORE_LOGGER_H_

#include "cppnetlib/core/log_stream.h"
#include "cppnetlib/core/time_anchor.h"

namespace cppnetlib {

// Maintains a LogStream and provides wrapper func
// Usage: 
//   Constructs a Logger object for logging need, 
//   Appends message to the buffer in it,
//   Flushes buffer to stdout when deconstructing.
class Logger {
 public:
  enum LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL };
  // Keeps basename of a file path
  struct SrcName {
    template<int N>  // nontype parameter
    SrcName(const char (&arr)[N]) : data_(arr), size_(N-1) {
      const char *slash = std::strrchr(data_, '/');
      if (slash) {
        data_ = slash + 1;
        size_ -= static_cast<int>(data_ - arr);
      }
    }

    explicit SrcName(const char *filename) : data_(filename) {
      const char *slash = std::strrchr(data_, '/');
      if (slash)
        data_ = slash + 1;
      size_ = static_cast<int>(strlen(data_));
    }

    const char *data_;
    int size_;
  };

  Logger(SrcName fname, int lineno) 
      : info_(INFO, 0, fname, lineno) {}
  Logger(SrcName fname, int lineno, LogLevel level) 
      : info_(level, 0, fname, lineno) {}
  Logger(SrcName fname, int lineno, LogLevel level, const char* func)
      : info_(level, 0, fname, lineno) { info_.stream_ << func << ' '; }
  Logger(SrcName fname, int lineno, bool to_abort)
      : info_(to_abort ? FATAL : ERROR, errno, fname, lineno) {}
  ~Logger();

  LogStream& stream() { return info_.stream_; }

  // ! Static functions !
  // set output and flush function for all logger objects
  // defaut output function write to stdout.
  using OuputFunc = void (*)(const char* msg, int len);
  using FlushFunc = void (*)();
  static void set_output(OuputFunc);
  static void set_flush(FlushFunc);
  static void set_loglevel(LogLevel);
  static LogLevel get_loglevel();

 private:
  /* why need another LogInfo struct? */
  struct LogInfo {
    LogInfo(Logger::LogLevel level, int old_errno, const SrcName& fname, 
            int lint);
    void Finish();

    LogStream stream_;
    TimeAnchor time_;
    LogLevel level_;
    SrcName basename_;
    int line_;
  };

  LogInfo info_;
};

extern Logger::LogLevel g_level;
inline Logger::LogLevel Logger::get_loglevel() { return g_level; };
const char* strerror_tl(int saved_errno);


#define LOG_TRACE if (Logger::get_loglevel() <= Logger::TRACE) \
    Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (Logger::get_loglevel() <= Logger::DEBUG) \
    Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO if (Logger::get_loglevel() <= Logger::INFO) \
    Logger(__FILE__, __LINE__).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()
#define LOG_SYSERR Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL Logger(__FILE__, __LINE__, true).stream()

} // namespace cppnetlib
# endif // CPPNETLIB_CORE_LOGGER_H_