#pragma once

#include <chrono>
#include <cstdint>

#include <fastrtps/rtps/common/Time_t.h>

#include "commkit/chrono.h"
#include "commkit/types.h"

namespace commkit
{

// Time conversions
// RTPS Time_t is the 32.32 (signed seconds, unsigned fraction) format
// clock::time_point (chrono::steady_clock) is (internally) int64_t nanoseconds
// "raw nanoseconds" is int64_t nanoseconds

// Constants
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

// Convert raw nanoseconds to Time_t
// Inexact conversion; increased precision
inline eprosima::fastrtps::rtps::Time_t toRtpsTime(std::int64_t nsec)
{
    if (nsec == NSEC_INVALID)
        return eprosima::fastrtps::rtps::c_TimeInvalid;

    eprosima::fastrtps::rtps::Time_t t;
    t.seconds = nsec / u10p9;
    // nsec % u10p9 is < u10p9, well under 2^32, so the 64-bit multiply can't overflow
    t.fraction = (nsec % u10p9 * u2p32 + u10p9 / 2) / u10p9;
    return t;
}

// Convert raw nanoseconds to clock::time_point
// Exact conversion
inline clock::time_point toTimePoint(const std::int64_t nsec)
{
    if (nsec == NSEC_INVALID)
        return TIME_POINT_INVALID;

    // Construct duration from int, then time_point from duration.
    clock::duration dur(nsec);
    clock::time_point tp(dur);
    return tp;
}

// Convert clock::time_point to raw nanoseconds
// (don't really need this one unless we use some other clock)
// Exact conversion
inline std::int64_t toInt64(const clock::time_point &tp)
{
    if (tp == TIME_POINT_INVALID)
        return NSEC_INVALID;

    return tp.time_since_epoch().count();
}

// Convert RTPS Time_t to clock::time_point
// Inexact conversion; loss of precision
inline clock::time_point toTimePoint(const eprosima::fastrtps::rtps::Time_t &t)
{
    if (t == eprosima::fastrtps::rtps::c_TimeInvalid)
        return TIME_POINT_INVALID;

    std::int64_t nsec = toInt64(t);
    return toTimePoint(nsec);
}

// Convert clock::time_point to RTPS Time_t
// Inexact conversion; increased precision
inline eprosima::fastrtps::rtps::Time_t toRtpsTime(const clock::time_point &tp)
{
    if (tp == TIME_POINT_INVALID)
        return eprosima::fastrtps::rtps::c_TimeInvalid;

    std::int64_t nsec = toInt64(tp);
    return toRtpsTime(nsec);
}

// Convert std::chrono::duration to RTPS Duration_t
// Other std::chrono units can be passed directly as 'd', and will be implicitly converted.
// ie, toRtpsDuration(std::chrono::milliseconds(500)) is fine.
inline eprosima::fastrtps::rtps::Duration_t toRtpsDuration(std::chrono::nanoseconds d)
{
    // Duration_t is a typedef of Time_t, static_cast in case that somehow changes
    return static_cast<eprosima::fastrtps::rtps::Duration_t>(toRtpsTime(d.count()));
}

} // namespace commkit
