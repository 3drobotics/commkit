#pragma once

/*
 * Convenience typedefs and definitions to save you valuable keystrokes.
 */

#include <cstdint>
#include <memory>

namespace commkit
{

class Publisher;
class Subscriber;

typedef std::shared_ptr<Publisher> PublisherPtr;
typedef std::shared_ptr<Subscriber> SubscriberPtr;

constexpr std::int64_t SEQUENCE_NUMBER_INVALID = 0xffffffff00000000ULL; // -1, 0
}
