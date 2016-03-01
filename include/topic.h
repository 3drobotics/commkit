#pragma once

#include <string>

namespace commkit
{

struct Topic {
    std::string name;
    std::string datatype;

    Topic(const std::string &n, const std::string &dt) : name(n), datatype(dt)
    {
    }
};

} // namespace commkit
