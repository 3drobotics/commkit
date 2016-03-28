#pragma once

#include <string>

#include <commkit/callback.h>
#include <commkit/topic.h>
#include <commkit/types.h>
#include <commkit/visibility.h>

#include <capnp/message.h>

namespace commkit
{

class NodeImpl;
class Publisher;
class PublisherImpl;

struct COMMKIT_API PublicationOpts {
    bool reliable;            //
    unsigned maxBlockingTime; // only relevant if reliable is true
    unsigned history; // number of samples to retain, to help late joining nodes to 'catch up'

    PublicationOpts() : reliable(true), maxBlockingTime(500), history(1)
    {
    }
};

class COMMKIT_API Publisher
{
public:
    ~Publisher();

    bool init(const PublicationOpts &opts);

    // XXX: make this const once TopicDataType.getName() is const
    std::string datatype();
    std::string name() const;

    bool reserve(uint8_t **b, size_t len);
    bool publishReserved(const uint8_t *b, size_t len);

    bool publish(capnp::MessageBuilder &mb);

    bool publish(const uint8_t *b, size_t len);
    unsigned matchedSubscribers() const;

    Callback<void(const PublisherPtr)> onSubscriberConnected;
    Callback<void(const PublisherPtr)> onSubscriberDisconnected;

private:
    Publisher(const Topic &t, std::shared_ptr<NodeImpl> n);

    std::unique_ptr<PublisherImpl> impl;

    friend class Node; // for private ctor
};

} // namespace commkit
