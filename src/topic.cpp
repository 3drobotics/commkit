#ifndef COMMKIT_NO_CAPNP
#include <commkit/topic.h>
#include <iomanip>
#include <sstream>
#include <iostream>

namespace commkit
{

std::string Topic::capn_type_id(capnp::Schema schema)
{
    uint64_t id = schema.getProto().getId();
    std::ostringstream idstr;
    idstr << std::hex << std::setw(16) << std::setfill('0') << id;
    return idstr.str();
}
}
#endif
