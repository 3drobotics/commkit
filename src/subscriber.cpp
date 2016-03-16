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

} // namespace commkit
