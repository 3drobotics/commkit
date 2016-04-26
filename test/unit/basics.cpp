
#include <gtest/gtest.h>
#include <commkit/commkit.h>

#include <chrono>
#include <thread>

TEST(BasicsTest, Basic)
{
    /*
     * Create a pub and sub on different nodes,
     * ensure we can send a trivial message between them.
     */

    commkit::Node n1, n2;
    EXPECT_TRUE(n1.init("node1"));
    EXPECT_TRUE(n2.init("node2"));

    auto t = commkit::Topic("T", "uint32_t", sizeof(uint32_t));

    auto pub = n1.createPublisher(t);
    commkit::PublicationOpts popts;
    popts.reliable = true;
    EXPECT_TRUE(pub->init(popts));

    auto sub = n2.createSubscriber(t);
    commkit::SubscriptionOpts sopts;
    sopts.reliable = true;
    EXPECT_TRUE(sub->init(sopts));

    // ensure they're connected
    unsigned tries = 100;
    while (pub->matchedSubscribers() == 0 && sub->matchedPublishers() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ASSERT_GT(tries--, 0);
    }

    uint32_t npub, nsub;

    for (unsigned i = 0; i < 10; ++i) {
        npub = i;
        EXPECT_TRUE(pub->publish(reinterpret_cast<const uint8_t *>(&npub), sizeof(npub)));

        // we are not validating receipt within a certain timeframe,
        // just want to give it time to arrive.
        // this is assumed to be way more than necessary.
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        commkit::Payload p;
        EXPECT_TRUE(sub->take(&p));
        EXPECT_EQ(p.len, sizeof(npub));
        memcpy(&nsub, p.bytes, sizeof(npub));
        EXPECT_EQ(npub, nsub);
    }
}
