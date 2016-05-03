#include <commkit/rtps.h>

#include <fastrtps/log/Log.h>

namespace commkit
{

void setRtpsLogVerbosity(RtpsLogVerbosity level)
{
    eprosima::Log::setVerbosity(static_cast<eprosima::LOG_VERBOSITY_LVL>(level));
}

} // namespace commkit
