#include <commkit/node.h>
#include "nodeimpl.h"

#include <map>
#include <mutex>

namespace commkit
{

Node::Node()
{
}

Node::~Node()
{
}

std::shared_ptr<NodeImpl> Node::getImpl(const NodeOpts &opts)
{
    /*
     * cache of NodeImpls, keyed by domainID.
     *
     * Main idea here is to avoid creating multiple participants
     * per domainID. Revisit if this turns out to be misguided.
     */

    static std::mutex mtx;
    static std::map<uint32_t, std::weak_ptr<NodeImpl>> cache;

    std::lock_guard<std::mutex> guard(mtx);

    auto impl = cache[opts.domainID].lock();
    if (!impl) {
        impl = std::make_shared<NodeImpl>();
        if (!impl || !impl->init(opts)) {
            return nullptr;
        }
        cache[opts.domainID] = impl;
    }
    return impl;
}

bool Node::init(const std::string &name)
{
    NodeOpts opts;
    opts.name = name;

    impl = getImpl(opts);
    return (impl != nullptr);
}

bool Node::init(const NodeOpts &opts)
{
    impl = getImpl(opts);
    return (impl != nullptr);
}

std::shared_ptr<Subscriber> Node::createSubscriber(const std::string &name,
                                                   const std::string &datatype)
{
    /*
     * Create and return a new subscriber.
     */

    // would rather use make_shared but Subscriber ctor is private
    return std::shared_ptr<Subscriber>(new Subscriber(name, datatype, impl));
}

std::shared_ptr<Publisher> Node::createPublisher(const std::string &name,
                                                 const std::string &datatype)
{
    /*
     * Create and return a new publisher.
     */

    // would rather use make_shared but Publisher ctor is private
    return std::shared_ptr<Publisher>(new Publisher(name, datatype, impl));
}

} // namespace commkit
