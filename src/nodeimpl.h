#pragma once

#include <string>

#include <commkit/node.h>

#include <fastrtps/participant/Participant.h>

namespace commkit
{

class NodeImpl
{
public:
    NodeImpl();
    ~NodeImpl();

    bool init(const NodeOpts &opts);

private:
    eprosima::fastrtps::Participant *part;

    friend class PublisherImpl;
    friend class SubscriberImpl;
};

} // namespace commkit
