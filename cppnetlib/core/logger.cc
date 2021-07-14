#include "cppnetlib/core/logger.h"

#include "cppnetlib/core/thisthread.h"

namespace cppnetlib {

__thread char t_errnobuf[512];

const char* strerror_tl(int saved_errno) {
  return strerror_r(saved_errno, t_errnobuf, sizeof(t_errnobuf));
}

void DefaultOutput(const char* msg, int len) { fwrite(msg, 1, len, stdout); }
void DefaultFlush() { fflush(stdout); }

Logger::LogLevel g_level = Logger::INFO;
Logger::OuputFunc g_output = DefaultOutput;
Logger::FlushFunc g_flush = DefaultFlush;

const char* level_name[6] = { "TRACE ", "DEBUG ", "INFO  ", 
                              "WARN  ", "ERROR ", "FATAL "};

// Members in Logger

void Logger::set_loglevel(LogLevel level) { g_level = level; }
void Logger::set_output(OuputFunc out) { g_output = out; }
void Logger::set_flush(FlushFunc flush) { g_flush = flush; }

Logger::~Logger() {
  info_.Finish();
  const LogStream::Buffer& residual_buf(stream().buffer());
  g_output(residual_buf.data(), residual_buf.length());
  if (info_.level_ == FATAL) {
    g_flush();
    abort();
  }
}

// Members in LogInfo

Logger::LogInfo::LogInfo(Logger::LogLevel level, int old_errno, 
                         const SrcName& fname, int line)
    : stream_(), time_(TimeAnchor::Now()), level_(level), basename_(fname), 
      line_(line) {
  stream_ << time_.FormatMicro() << ' ';
  thisthread::tid();
  stream_.Append(thisthread::tid_str(), thisthread::tid_str_len());
  stream_.Append(level_name[level], 6);
  if (old_errno)
    stream_ << strerror_tl(old_errno) << " errno=" << old_errno << ") ";
}

void Logger::LogInfo::Finish() {
  stream_ << " - ";
  stream_.Append(basename_.data_, basename_.size_);
  stream_ << ':' << line_ << '\n';
}

} // namespace cppnetlib