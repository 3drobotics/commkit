#include <commkit/subscriber.h>
#include "subscriberimpl.h"
#include "nodeimpl.h"

namespace commkit
{

Subscriber::Subscriber(std::shared_ptr<NodeImpl> n) : impl(new SubscriberImpl(n, this))
{
}

Subscriber::~Subscriber()
{
}

bool Subscriber::init(const Topic &t, const SubscriptionOpts &opts)
{
    return impl->init(t, opts);
}

bool Subscriber::peek(Payload *p)
{
    return impl->peek(p);
}

int Subscriber::take(Payload *p)
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
