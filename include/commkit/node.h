#pragma once

#include <string>
#include <vector>
#include <memory>

#include "subscriber.h"
#include "publisher.h"
#include "visibility.h"

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

    std::shared_ptr<Subscriber> createSubscriber(const std::string &name,
                                                 const std::string &datatype);
    std::shared_ptr<Publisher> createPublisher(const std::string &name,
                                               const std::string &datatype);

private:
    static std::shared_ptr<NodeImpl> getImpl(const NodeOpts &opts);

    std::shared_ptr<NodeImpl> impl;
};

} // namespace commkit
