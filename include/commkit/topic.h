#pragma once

#include <string>

#ifndef COMMKIT_NO_CAPNP
#include <commkit/visibility.h>
#include <capnp/schema.h>
#endif

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

#ifndef COMMKIT_NO_CAPNP
    static std::string COMMKIT_API capn_type_id(capnp::Schema schema);

    template <typename T>
    static Topic capn(const std::string &n)
    {
        return Topic(n, capn_type_id(capnp::Schema::from<T>()), 1024);
    }
#endif
};

} // namespace commkit
