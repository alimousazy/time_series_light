#pragma once

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

time_t get_week_start(time_t raw_time);
time_t incr_week(time_t s, int length);
