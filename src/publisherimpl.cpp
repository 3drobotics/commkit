#include "publisherimpl.h"
#include "nodeimpl.h"
#include "bytebuftopic.h"

#include <cassert>

#include <fastrtps/Domain.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastrtps/attributes/PublisherAttributes.h>

namespace commkit
{

PublisherImpl::PublisherImpl(std::shared_ptr<NodeImpl> n, Publisher *p)
    : matchedSubs(0), reserved(false), node(n), pub(p)
{
}

PublisherImpl::~PublisherImpl()
{
    if (frpub != nullptr) {
        eprosima::fastrtps::Domain::removePublisher(frpub);
    }
}

bool PublisherImpl::init(const Topic &t, const PublicationOpts &opts)
{
    eprosima::fastrtps::PublisherAttributes pa;
    pa.topic.topicDataType = t.datatype;
    pa.topic.topicKind = NO_KEY;
    pa.topic.topicName = t.name;
    pa.times.heartbeatPeriod.fraction = 4294967 * 200; // ~200 millis
    // XXX: configure history

    if (opts.reliable) {
        pa.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
    } else {
        pa.qos.m_reliability.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
    }

    // ugh, payload size must be defined
    assert(opts.maxPayloadSize > 0);

    topicName = t.name;
    topicDataType.setName(t.datatype.c_str());
    topicDataType.setSize(opts.maxPayloadSize);

    // fast-rtps requires datatype to be regsitered before we can
    // create the publisher.

    eprosima::fastrtps::TopicDataType *tdt;
    if (!eprosima::fastrtps::Domain::getRegisteredType(node->part, t.datatype.c_str(), &tdt)) {
        eprosima::fastrtps::Domain::registerType(node->part, &topicDataType);
    }

    frpub = eprosima::fastrtps::Domain::createPublisher(node->part, pa, this);
    return (frpub != nullptr);
}

bool PublisherImpl::reserve(uint8_t **b, size_t len)
{
    /*
     * Get a pointer to a buffer to serialize an outgoing message into.
     * Must be followed by a call to commit() to actually send the data.
     *
     * Allows the serializer to write directly to the buffer that will
     * be sent over the wire, so avoids an extra copy step.
     */

    if (len > topicDataType.m_typeSize) {
        return false;
    }

    if (!topicData.ensureCap(len)) {
        return false;
    }

    *b = topicData.buf;
    reserved = true;
    return true;
}

bool PublisherImpl::publishReserved(const uint8_t *b, size_t len)
{
    /*
     * Send data that has been written to topicData via reserve().
     */

    assert(reserved && "publishReserved() called without first calling reserve()");
    reserved = false;

    // sanity check, make sure caller is passing back reserved data
    if (b != topicData.buf) {
        return false;
    }

    if (!matchedSubs) {
        return false; // don't bother if nobody is listening
    }

    if (len > topicDataType.m_typeSize || len > topicData.cap) {
        return false;
    }

    topicData.len = len;
    return frpub->write(&topicData);
}

bool PublisherImpl::publish(const uint8_t *b, size_t len)
{
    /*
     * Publish some bytes.
     * Callers must have already serialized their data into b.
     *
     * See reserve()/commit() for a zero-copy API.
     */

    assert(!reserved && "publish() called while reserve() is still outstanding");

    if (!matchedSubs) {
        return false; // don't bother if nobody is listening
    }

    if (len > topicDataType.m_typeSize) {
        return false;
    }

    if (!topicData.write(b, len)) {
        return false;
    }

    return frpub->write(&topicData);
}

void PublisherImpl::onPublicationMatched(eprosima::fastrtps::Publisher *,
                                         eprosima::fastrtps::rtps::MatchingInfo &info)
{
    switch (info.status) {
    case MATCHED_MATCHING:
        matchedSubs++;
        pub->onSubscriberConnected(pub);
        break;

    case REMOVED_MATCHING:
        matchedSubs--;
        pub->onSubscriberDisconnected(pub);
    }
}

} // namespace commkit
