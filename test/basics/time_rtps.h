#pragma once

// This is a hack to get some control over type-helpers for these test
// programs. It is likely that much of this should be accessible to
// other program, perhaps layered underneath or next to commkit in the
// grand scheme.
//
// Application code is expected to use std::chrono for most if not all
// timing.
//
// Definitions here support usage of and conversion of time values
// between three domains:
//  * raw nanoseconds since "epoch" (int64_t)
//  * RTPS time (Time_t)
//  * std::chrono time (time_point and duration)
//
// Everything in here assumes the underlying system's monotonic clock.
// std::chrono time explicitly uses it, and the "epoch" for RTPS time
// and raw nanoseconds is assumed to be the same for all three times
// domains.

#include <chrono>

#include <fastrtps/rtps/common/Time_t.h>

// -----8<----- abridged copy from commkit/include/commkit/chrono.h

typedef std::chrono::steady_clock TestClock;

const TestClock::duration DURATION_INVALID = TestClock::duration(-1);
const TestClock::time_point TIME_POINT_INVALID = TestClock::time_point(DURATION_INVALID);

// -----8<----- end copy

// -----8<----- abridged copy from commkit/src/chronoimpl.h

constexpr std::int64_t NSEC_INVALID = -1;

// Conversion factors
constexpr std::uint64_t u10p9 = 1000000000ll;
constexpr std::uint64_t u2p32 = 1ull << 32;

// Convert Time_t to raw nanoseconds
// Inexact conversion; loss of precision
inline std::int64_t toInt64(const eprosima::fastrtps::rtps::Time_t &t)
{
    if (t == eprosima::fastrtps::rtps::c_TimeInvalid)
        return NSEC_INVALID;

    // t.seconds is 32 bits and u10p9 fits in 32 bits, so the first
    // (64 bit) multiply can't overflow. Similar for the t.fraction
    // multiply; then the add can't overflow because the result of the
    // multiply is always small enough.
    std::int64_t nsec = t.seconds * u10p9 + (t.fraction * u10p9 + u2p32 / 2) / u2p32;
    return nsec;
}

// Convert raw nanoseconds to TestClock::time_point
// Exact conversion
inline TestClock::time_point toTimePoint(const std::int64_t nsec)
{
    if (nsec == NSEC_INVALID)
        return TIME_POINT_INVALID;

    // Construct duration from int, then time_point from duration.
    TestClock::duration dur(nsec);
    TestClock::time_point tp(dur);
    return tp;
}

// Convert RTPS Time_t to TestClock::time_point
// Inexact conversion; loss of precision
inline TestClock::time_point toTimePoint(const eprosima::fastrtps::rtps::Time_t &t)
{
    if (t == eprosima::fastrtps::rtps::c_TimeInvalid)
        return TIME_POINT_INVALID;

    std::int64_t nsec = toInt64(t);
    return toTimePoint(nsec);
}

// -----8<----- end copy

// other conversions (not in commkit)

// TestClock::duration to raw nanoseconds
// Exact conversion
inline int64_t toInt64(const TestClock::duration &d)
{
    if (d == DURATION_INVALID)
        return NSEC_INVALID;

    return std::chrono::duration_cast<std::chrono::nanoseconds>(d).count();
}

// TestClock::duration to double seconds
// Inexact conversion, depends on value (!); this is intended for logging.
inline double toDouble(const TestClock::duration &d)
{
    return std::chrono::duration_cast<std::chrono::duration<double>>(d).count();
}

// double seconds to RTPS Time_t
// Inexact conversion, depends on value (!); this is intended for things like
// initializing durations or timeouts.
inline eprosima::fastrtps::rtps::Time_t toTime(double t)
{
    if (t == -1.0)
        return c_TimeInfinite;
    double frac = t - int(t);
    return eprosima::fastrtps::rtps::Time_t(t, frac * (1ull << 32));
}
