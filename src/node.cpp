#include <commkit/node.h>
#include "nodeimpl.h"
#include "publisherimpl.h"
#include "subscriberimpl.h"

#include <map>
#include <mutex>
#include <cassert>

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

Node Node::find(uint32_t domainID)
{
    /*
     * Create a Node from an existing impl for 'domainID'.
     *
     * This can be more convenient than creating a new node & then initializing.
     * Because it has no way of reporting an error, it asserts if a node
     * has not already been created for the given domain.
     */

    NodeOpts opts;
    opts.domainID = domainID;
    auto i = getImpl(opts);
    assert(i != nullptr && "didn't find existing node");
    return Node(i);
}

std::shared_ptr<Subscriber> Node::createSubscriber(const Topic &t)
{
    /*
     * Create and return a new subscriber.
     */

    // would rather use make_shared but Subscriber ctor is private
    auto sub = std::shared_ptr<Subscriber>(new Subscriber(t, impl));
    sub->impl->setSubscriber(sub);
    return sub;
}

std::shared_ptr<Publisher> Node::createPublisher(const Topic &t)
{
    /*
     * Create and return a new publisher.
     */

    // would rather use make_shared but Publisher ctor is private
    auto pub = std::shared_ptr<Publisher>(new Publisher(t, impl));
    pub->impl->setPublisher(pub);
    return pub;
}

} // namespace commkit
