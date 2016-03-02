#pragma once

#include <string>

#include "visibility.h"

namespace commkit
{

struct COMMKIT_API Topic {
    std::string name;
    std::string datatype;

    Topic(const std::string &n, const std::string &dt) : name(n), datatype(dt)
    {
    }
};

} // namespace commkit
