#include <commkit/subscriber.h>
#include "subscriberimpl.h"
#include "nodeimpl.h"

namespace commkit
{

Subscriber::Subscriber(const Topic &t, std::shared_ptr<NodeImpl> n) : impl(new SubscriberImpl(t, n))
{
}

Subscriber::~Subscriber()
{
}

bool Subscriber::init(const SubscriptionOpts &opts)
{
    return impl->init(opts);
}

bool Subscriber::peek(Payload *p)
{
    return impl->peek(p);
}

bool Subscriber::take(Payload *p)
{
    return impl->take(p);
}

void Subscriber::waitForMessage()
{
    return impl->waitForMessage();
}

unsigned Subscriber::matchedPublishers() const
{
    return impl->matchedPublishers();
}

std::string Subscriber::datatype()
{
    return impl->datatype();
}

std::string Subscriber::name() const
{
    return impl->name();
}

#ifndef COMMKIT_NO_CAPNP
capnp::FlatArrayMessageReader Payload::toReader(bool *ok)
{
    /*
     * Use FlatArrayMessageReader since that's what Publisher uses for now.
     * Ultimately want a strategy that involves less copying.
     */

    using capnp::word;

    if (len % sizeof(word) == 0) {
        if (ok) {
            *ok = true;
        }
        auto wb =
            kj::ArrayPtr<const word>(reinterpret_cast<const word *>(bytes), len / sizeof(word));
        return capnp::FlatArrayMessageReader(wb);
    }

    if (ok) {
        *ok = false;
    }
    return capnp::FlatArrayMessageReader(kj::ArrayPtr<const word>(nullptr, (size_t)0));
}
#endif

} // namespace commkit
