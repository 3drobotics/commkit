#pragma once

#include <chrono>

namespace commkit
{

// this selects (at compile time) what clock is used throughout commkit
// (mainly clock::time_point and clock::duration)
typedef std::chrono::steady_clock clock;

const clock::time_point TIME_POINT_INVALID = clock::time_point(clock::duration(-1));

// Convert to floating-point representation (in seconds) of a clock::duration.
// Useful for logging/debugging, but should *not* be used for time calculations;
// loss of precision depending on duration value can happen.
inline double toDouble(clock::duration d)
{
    return std::chrono::duration_cast<std::chrono::duration<double>>(d).count();
}

// would like an implicit conversion to double from commkit::duration for iostream use
}
