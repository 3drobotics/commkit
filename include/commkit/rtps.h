#pragma once

/*
 * Utilities (hopefully very few) to configure/manipulate
 * the underlying RTPS implementation.
 */

namespace commkit
{

/*
 * Currently these map directly to Fast-RTPS log levels,
 * consider reorg'ing if necessary in the future.
 */
enum RtpsLogVerbosity {
    RTPS_VERBOSITY_QUIET,
    RTPS_VERBOSITY_ERROR,
    RTPS_VERBOSITY_WARNING,
    RTPS_VERBOSITY_INFO
};

void setRtpsLogVerbosity(RtpsLogVerbosity level);

} // namespace commkit
