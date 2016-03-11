#pragma once

#include <commkit/publisher.h>
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
    PublisherImpl(const Topic &t, std::shared_ptr<NodeImpl> n);
    ~PublisherImpl();

    void setPublisher(std::shared_ptr<Publisher> p)
    {
        // Only expected to be called via Node during Publisher construction.
        pub = p;
    }

    bool init(const PublicationOpts &opts);

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

    std::weak_ptr<Publisher> pub;
};

} // namespace commkit
