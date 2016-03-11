#pragma once

#include <string>
#include <vector>
#include <memory>

#include <commkit/topic.h>
#include <commkit/subscriber.h>
#include <commkit/publisher.h>
#include <commkit/visibility.h>

namespace commkit
{

class NodeImpl;

/*
 * Options to configure a Node.
 */
struct COMMKIT_API NodeOpts {
    std::string name;
    uint32_t domainID;
    std::vector<std::string> unicastLocators;   // where to send runtime data
    std::vector<std::string> multicastLocators; // where to send discovery data

    static constexpr uint32_t DefaultDomain = 80;

    NodeOpts() : domainID(DefaultDomain)
    {
    }
};

/*
 * A Node represents a Node.
 */
class COMMKIT_API Node
{
public:
    Node();
    ~Node();

    bool init(const std::string &name);
    bool init(const NodeOpts &opts);

    std::shared_ptr<Subscriber> createSubscriber(const Topic &t);
    std::shared_ptr<Publisher> createPublisher(const Topic &t);

private:
    static std::shared_ptr<NodeImpl> getImpl(const NodeOpts &opts);

    std::shared_ptr<NodeImpl> impl;
};

} // namespace commkit
