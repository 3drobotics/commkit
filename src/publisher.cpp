#include <commkit/publisher.h>
#include "publisherimpl.h"
#include "nodeimpl.h"

namespace commkit
{

Publisher::Publisher(const Topic &t, std::shared_ptr<NodeImpl> n) : impl(new PublisherImpl(t, n))
{
}

Publisher::~Publisher()
{
}

bool Publisher::init(const PublicationOpts &opts)
{
    return impl->init(opts);
}

std::string Publisher::datatype()
{
    return impl->datatype();
}

std::string Publisher::name() const
{
    return impl->name();
}

bool Publisher::reserve(uint8_t **b, size_t len)
{
    return impl->reserve(b, len);
}

bool Publisher::publishReserved(const uint8_t *b, size_t len)
{
    return impl->publishReserved(b, len);
}

#ifndef COMMKIT_NO_CAPNP
bool Publisher::publish(capnp::MessageBuilder &mb)
{
    /*
     * For now, we use messageToFlatArray() but this is not ideal,
     * as it involves a copy.
     *
     * Ideally, we'd either
     * a) write to pre-allocated space to begin with,
     *  but we don't always know how much space to reserve
     * b) write the segments out as fragments, as suggested in
     * http://stackoverflow.com/questions/32041315/how-to-send-capn-proto-message-over-zmq
     *
     * We may also want to use a packed variant, not sure yet.
     */

    auto words = capnp::messageToFlatArray(mb);
    auto bytes = words.asBytes();
    return impl->publish(bytes.begin(), bytes.size());
}
#endif

bool Publisher::publish(const uint8_t *b, size_t len)
{
    return impl->publish(b, len);
}

unsigned Publisher::matchedSubscribers() const
{
    return impl->matchedSubscribers();
}

} // namespace commkit
