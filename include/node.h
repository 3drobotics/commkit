#pragma once

#include <string>
#include <vector>
#include <memory>

#include "subscriber.h"
#include "publisher.h"

namespace commkit
{

class NodeImpl;

/*
 * Options to configure a Node.
 */
struct NodeOpts {
    std::string name;
    uint32_t domainID;
    std::vector<std::string> unicastLocators;   // where to send runtime data
    std::vector<std::string> multicastLocators; // where to send discovery data

    NodeOpts() : domainID(80)
    {
    }
};

/*
 * A Node represents a Node.
 */
class Node
{
public:
    Node();
    ~Node();

    bool init(const std::string &name);
    bool init(const NodeOpts &opts);

    std::shared_ptr<Subscriber> subscribe(const Topic &t, const SubscriptionOpts &opts);
    std::shared_ptr<Publisher> publish(const Topic &t, const PublicationOpts &opts);

private:
    static std::shared_ptr<NodeImpl> getImpl(const NodeOpts &opts);

    std::shared_ptr<NodeImpl> impl; // all nodes in this process point to the same impl
};

} // namespace commkit
