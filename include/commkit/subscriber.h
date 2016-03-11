#pragma once

#include <memory>
#include <string>

#include "callback.h"
#include "visibility.h"

namespace commkit
{

class NodeImpl;
class Subscriber;
class SubscriberImpl;

/*
 * Options to configure a Subscriber.
 */
struct COMMKIT_API SubscriptionOpts {
    bool reliable;
    int timeBasedFilterHere; // todo
    unsigned history;
    unsigned maxPayloadSize; // ugh, this is currently required by fast-rtps. need a workaround.

    SubscriptionOpts() : reliable(false), timeBasedFilterHere(0), history(1), maxPayloadSize(0)
    {
    }
};

/*
 * Type to return received data.
 * Subscriber::peek() and Subscriber::take() populate this with
 * a pointer to internally received data, in order to avoid an
 * additional copy step.
 */
struct COMMKIT_API Payload {
    uint8_t *bytes;
    size_t len;

    Payload() : bytes(nullptr), len(0)
    {
    }
};

/*
 * Subscriber subscribes to a topic described by a name and datatype.
 * It reports new
 */
class COMMKIT_API Subscriber
{
public:
    ~Subscriber();

    bool init(const SubscriptionOpts &opts);

    bool peek(Payload *p);
    int take(Payload *p);
    unsigned matchedPublishers() const;

    // XXX: make this const once TopicDataType.getName() is const
    std::string datatype();
    std::string name() const;

    Callback<void(const Subscriber *)> onPublisherConnected;
    Callback<void(const Subscriber *)> onPublisherDisconnected;
    Callback<void(Subscriber *)> onMessage;

private:
    Subscriber(const std::string &name, const std::string &datatype, std::shared_ptr<NodeImpl> n);

    std::unique_ptr<SubscriberImpl> impl;

    friend class Node; // for private ctor
};

} // namespace commkit
