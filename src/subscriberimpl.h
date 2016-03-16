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
    SubscriberImpl(const Topic &t, std::shared_ptr<NodeImpl> n);
    ~SubscriberImpl();

    void setSubscriber(std::shared_ptr<Subscriber> s)
    {
        // Only expected to be called via Node during Subscriber construction.
        sub = s;
    }

    bool init(const SubscriptionOpts &opts);

    bool peek(Payload *p);
    bool take(Payload *p);

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

    std::weak_ptr<Subscriber> sub;
};

} // namespace commkit
