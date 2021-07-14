#include <cstdio>

#include "cppnetlib/core/time_anchor.h"

using namespace cppnetlib;

int main() {
  TimeAnchor start(TimeAnchor::Now());
  printf("now -> %s\n", start.ToStr().c_str());
  printf("The micro format time: %s\n", start.FormatMicro().c_str());
  printf("The seconds format time: %s\n", start.FormatSeconds().c_str());

  // some busy task
  for (int i = 0; i < 10000; i++) {}

  // show time interval
  printf("now time interval: %f\n", TimeDiff(TimeAnchor::Now(), start));
}