#pragma once

#include <commkit/subscriber.h>
#include "nodeimpl.h"
#include "bytebuftopic.h"

#include <fastrtps/rtps/common/all_common.h>
#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/subscriber/SubscriberListener.h>

namespace commkit
{

class SubscriberImpl : public eprosima::fastrtps::SubscriberListener
{
public:
    SubscriberImpl(std::shared_ptr<NodeImpl> n, Subscriber *s) : matchedPubs(0), node(n), sub(s)
    {
    }
    ~SubscriberImpl();

    bool init(const Topic &t, const SubscriptionOpts &opts);

    bool peek(Payload *p);
    int take(Payload *p);

    unsigned matchedPublishers() const
    {
        return matchedPubs;
    }

    // XXX: make this const once TopicDataType.getName() is const
    std::string datatype()
    {
        return std::string(topicDataType.getName());
    }

    std::string name() const
    {
        return topicName;
    }

    void onSubscriptionMatched(eprosima::fastrtps::Subscriber *,
                               eprosima::fastrtps::rtps::MatchingInfo &info);
    void onNewDataMessage(eprosima::fastrtps::Subscriber *);

private:
    eprosima::fastrtps::Subscriber *frsub;
    unsigned matchedPubs;
    std::shared_ptr<NodeImpl> node;

    std::string topicName;
    ByteBufTopicData topicData;
    ByteBufTopicDataType topicDataType;

    Subscriber *sub;
};

} // namespace commkit
