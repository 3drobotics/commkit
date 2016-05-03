#pragma once

#include <stdint.h>

#if (defined __MACH__ && defined __APPLE__)
typedef int clockid_t;
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
#else
#include <time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern uint64_t clock_gettime_ns(clockid_t clock_id);
extern void clock_settime_ns(clockid_t clock_id, uint64_t t_ns);
extern const char *clock_tostr_r(uint64_t t_ns, char *buf);
extern const char *clock_gettime_str_r(clockid_t clock_id, char *buf);

#ifdef __cplusplus
};
#endif
