#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <time.h>
char *get_timestring(const struct tm * timeptr);
void epoch_to_time(struct tm *times, uint64_t ts);
uint64_t epoch_from_time(const struct tm *times);

#ifdef __cplusplus
}
#endif
