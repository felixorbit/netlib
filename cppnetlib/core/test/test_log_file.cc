#include <unistd.h> // for basename(3)

#include "cppnetlib/core/log_file.h"
#include "cppnetlib/core/logger.h"

using namespace cppnetlib;

std::unique_ptr<LogFile> g_logfile;


void OutputFunc(const char* msg, int len) {
  g_logfile->Append(msg, len);
}

void FlushFunc() {
  g_logfile->Flush();
}

int main(int argc, char* argv[]) {
  char process_name[256] = { '\0' };
  strncpy(process_name, argv[0], sizeof(process_name) - 1);

  // use g_logfile to manage LogFile object
  // rolling file every 200K bytes
  g_logfile.reset(new LogFile(::basename(process_name), 200*1000));
  
  Logger::set_output(OutputFunc);
  Logger::set_flush(FlushFunc);

  // rubbish log
  char rubbish[66] = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

  for (int i = 0; i < 10000; i++) {
    LOG_INFO << rubbish << i;
    // sleep 1 millisecond
    usleep(1000);
  }
  (void)argc;
}