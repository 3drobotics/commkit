#pragma once

/*
 * Convenience typedefs to save you valuable keystrokes.
 */

#include <memory>

namespace commkit
{

class Publisher;
class Subscriber;

typedef std::shared_ptr<Publisher> PublisherPtr;
typedef std::shared_ptr<Subscriber> SubscriberPtr;
}
