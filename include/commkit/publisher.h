#pragma once

#include <memory>

#include "topic.h"
#include "callback.h"
#include "visibility.h"

namespace commkit
{

class NodeImpl;
class Publisher;
class PublisherImpl;

struct COMMKIT_API PublicationOpts {
    bool reliable;            //
    unsigned maxBlockingTime; // only relevant if reliable is true
    unsigned history; // number of samples to retain, to help late joining nodes to 'catch up'
    unsigned maxPayloadSize; // ugh, this is currently required by fast-rtps. need a workaround.

    PublicationOpts() : reliable(true), maxBlockingTime(500), history(1), maxPayloadSize(0)
    {
    }
};

class COMMKIT_API Publisher
{
public:
    ~Publisher();

    // XXX: make this const once TopicDataType.getName() is const
    std::string datatype();
    std::string name() const;

    bool reserve(uint8_t **b, size_t len);
    bool publishReserved(const uint8_t *b, size_t len);

    bool publish(const uint8_t *b, size_t len);
    unsigned matchedSubscribers() const;

    Callback<void(const Publisher *)> onSubscriberConnected;
    Callback<void(const Publisher *)> onSubscriberDisconnected;

private:
    Publisher(std::shared_ptr<NodeImpl> n);
    bool init(const Topic &t, const PublicationOpts &opts);

    std::unique_ptr<PublisherImpl> impl;

    friend class Node; // for private ctor
};

} // namespace commkit
