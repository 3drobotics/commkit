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

std::shared_ptr<Subscriber> Node::subscribe(const Topic &t, const SubscriptionOpts &opts)
{
    /*
     * Create and return a new subscriber.
     */

    // would rather use make_shared but Subscriber ctor is private
    std::shared_ptr<Subscriber> s(new Subscriber(impl));
    if (!s->init(t, opts)) {
        cout << "failed to create subscriber :(" << endl;
        return nullptr; // XXX be helpful
    }
    return s;
}

std::shared_ptr<Publisher> Node::publish(const Topic &t, const PublicationOpts &opts)
{
    /*
     * Create and return a new publisher.
     */

    // would rather use make_shared but Publisher ctor is private
    std::shared_ptr<Publisher> p(new Publisher(impl));
    if (!p->init(t, opts)) {
        cout << "failed to create publisher :(" << endl;
        return nullptr; // XXX be helpful
    }
    return p;
}

} // namespace commkit
