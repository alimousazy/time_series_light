#include "time_convert.h"

time_t get_week_start(time_t raw_time) {
  struct tm *c_time;
  setenv("TZ", "UTC", 1);
  tzset();
  if(raw_time == -1) 
    goto error;
  c_time = gmtime(&raw_time);
  if(c_time == NULL) 
    goto error;
  c_time->tm_mday = c_time->tm_mday - c_time->tm_wday;
  c_time->tm_hour = 0;
  c_time->tm_min = 0;
  c_time->tm_sec = 0;
  c_time->tm_wday = 0;
  return mktime(c_time);
error:
  return -1;
}

time_t incr_week(time_t s, int length) {
  return s + length;
}

