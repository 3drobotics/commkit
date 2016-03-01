#pragma once

#include "publisher.h"
#include "nodeimpl.h"
#include "bytebuftopic.h"

#include <fastrtps/participant/Participant.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/publisher/PublisherListener.h>

namespace commkit
{

class PublisherImpl : public eprosima::fastrtps::PublisherListener
{
public:
    PublisherImpl(std::shared_ptr<NodeImpl> n, Publisher *p);
    ~PublisherImpl();

    bool init(const Topic &t, const PublicationOpts &opts);

    bool reserve(uint8_t **b, size_t len);
    bool publishReserved(const uint8_t *b, size_t len);

    bool publish(const uint8_t *b, size_t len);

    unsigned matchedSubscribers() const
    {
        return matchedSubs;
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

    void onPublicationMatched(eprosima::fastrtps::Publisher *,
                              eprosima::fastrtps::rtps::MatchingInfo &info);

private:
    eprosima::fastrtps::Publisher *frpub;
    int matchedSubs;
    bool reserved; // was reserve() called?
    std::shared_ptr<NodeImpl> node;

    std::string topicName;
    ByteBufTopicData topicData;
    ByteBufTopicDataType topicDataType;

    Publisher *pub;
};

} // namespace commkit
