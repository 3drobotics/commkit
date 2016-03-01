#include "publisher.h"
#include "publisherimpl.h"
#include "nodeimpl.h"

namespace commkit
{

Publisher::Publisher(std::shared_ptr<NodeImpl> n) : impl(new PublisherImpl(n, this))
{
}

Publisher::~Publisher()
{
}

bool Publisher::init(const Topic &t, const PublicationOpts &opts)
{
    return impl->init(t, opts);
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

bool Publisher::publish(const uint8_t *b, size_t len)
{
    return impl->publish(b, len);
}

unsigned Publisher::matchedSubscribers() const
{
    return impl->matchedSubscribers();
}

} // namespace commkit
