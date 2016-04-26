#pragma once

#include <cstdint>

#include <fastrtps/rtps/common/SequenceNumber.h>

#include "commkit/types.h"

namespace commkit
{

inline std::int64_t toInt64(const eprosima::fastrtps::rtps::SequenceNumber_t &seq)
{
    std::int64_t i = seq.high;
    i = (i << 32) | seq.low;
    return i;
}

} // namespace commkit
