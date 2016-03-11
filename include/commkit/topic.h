#pragma once

#include <string>

namespace commkit
{

struct Topic {
    std::string name;
    std::string datatype;
    size_t maxPayloadSize; // ugh, this is currently required by fast-rtps. need a workaround.

    Topic(const std::string &n, const std::string &dt, size_t maxSz)
        : name(n), datatype(dt), maxPayloadSize(maxSz)
    {
    }
};

} // namespace commkit
