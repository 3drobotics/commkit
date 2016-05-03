
#include <stdint.h>
#include <stdio.h>

#if (defined __MACH__ && defined __APPLE__)
#include <mach/mach_time.h>
#include <sys/time.h>
#else
#include <time.h>
#endif

#include "time_util.h"

/* Get current wall-clock time and return it in nanoseconds since the Unix
 * epoch.
 *
 * CLOCK_REALTIME should be used to get the system's best guess at real time.
 * CLOCK_MONOTONIC should be used when jumps in time would cause trouble.
 */
uint64_t clock_gettime_ns(clockid_t clock_id)
{
#if (!defined __MACH__ || !defined __APPLE__)
    struct timespec ts;
    clock_gettime(clock_id, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
#else
    static struct mach_timebase_info tb_info = {0, 0};
    uint64_t now_ns = (uint64_t)(-1);
    if (tb_info.denom == 0)
        mach_timebase_info(&tb_info);
    if (clock_id == CLOCK_MONOTONIC) {
        now_ns = (mach_absolute_time() * tb_info.numer) / tb_info.denom;
    } else if (clock_id == CLOCK_REALTIME) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        now_ns = (tv.tv_sec * 1000000000ULL) + tv.tv_usec * 1000;
    }
    return now_ns;
#endif
}

/* Set clock.
 * This is usually used with CLOCK_REALTIME.
 */
void clock_settime_ns(clockid_t clock_id, uint64_t t_ns)
{
#if (!defined __MACH__ || !defined __APPLE__)
    struct timespec ts;
    ts.tv_sec = t_ns / 1000000000;
    ts.tv_nsec = t_ns % 1000000000;
    clock_settime(clock_id, &ts);
#endif
}

/* buf must be at least 24 characters */
const char *clock_tostr_r(uint64_t t_ns, char *buf)
{
    time_t s = t_ns / 1000000000;
    unsigned ns = t_ns % 1000000000;
    struct tm now_tm;
    size_t num_chars;

    localtime_r(&s, &now_tm);

    /* %F is %Y-%m-%d (4+1+2+1+2=10 chars)
     * %T is %H:%M:%S (2+1+2+1+2=8 chars)
     * total is then 10+1+8+1=20 chars, 21 with EOS */
    num_chars = strftime(buf, 21, "%F %T,", &now_tm);

    /* append 3 chars, rounding to nearest millisecond */
    sprintf(buf + num_chars, "%03d", (ns + 500000) / 1000000);

    return buf;
}

/* Get a clock and return as human-readable string. */
const char *clock_gettime_str_r(clockid_t clock_id, char *buf)
{
    return clock_tostr_r(clock_gettime_ns(clock_id), buf);
}
