#include <errno.h>
#include <getopt.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdlib>
#include <cstring>

#include "fastrtps/log/Log.h"

#include <commkit/node.h>
#include <commkit/subscriber.h>

#include "time_util.h"

#include "topic_data.h"
#include "test_config.h"
#include "resources.h"

static const char *prog = "test_sub_commkit";

uint64_t zero_ns;
uint64_t print_interval_ns;
uint64_t print_next_ns;

static Resources resources;

// sequence number in last packet received
static uint32_t last_seq = UINT32_MAX;

static void onConnect(const commkit::SubscriberPtr sub)
{
    printf("publisher connected (%u)\n", sub->matchedPublishers());
}

static void onDisconnect(const commkit::SubscriberPtr sub)
{
    printf("publisher disconnected (%u)\n", sub->matchedPublishers());
}

static void onMessage(commkit::SubscriberPtr sub)
{
    commkit::Payload payload;
    while (sub->take(&payload)) {

        if (payload.len != sizeof(TopicData)) {
            printf("message wrong length (expected %u, got %u)\n", (unsigned)sizeof(TopicData),
                   (unsigned)(payload.len));
            continue;
        }

        TopicData topic_data;
        memcpy(&topic_data, payload.bytes, sizeof(TopicData));

        // look for and print gaps in sequence number
        uint32_t sequence = topic_data.sequence;
        if (last_seq != UINT32_MAX && (sequence - last_seq) > 1) {
            printf("gap: %u %50c\n", sequence - last_seq - 1, ' ');
        }
        last_seq = sequence;

        uint64_t now_ns = clock_gettime_ns(CLOCK_MONOTONIC);
        if (now_ns >= print_next_ns) {
            resources.sample();
            double cpu = resources.cpu_load();
            uint64_t msec = (now_ns - zero_ns + 500000) / 1000000;
            printf("%-18s %6u.%03u: %6u %5.1f%%\n", prog, (unsigned)(msec / 1000),
                   (unsigned)(msec % 1000), sequence, cpu * 100);
            print_next_ns += print_interval_ns;
        }
    }
}

int main(int argc, char *argv[])
{
    printf("built %s %s\n", __DATE__, __TIME__);

    TestConfig::Config config;
    if (!TestConfig::parse_args(argc, argv, config)) {
        TestConfig::usage(prog);
    }

    eprosima::Log::setVerbosity(eprosima::VERB_ERROR);

    // zero time is next one-second boundary that is least 0.1 sec away
    zero_ns = clock_gettime_ns(CLOCK_MONOTONIC) + 1100000000; // 1.1 sec from now
    zero_ns = zero_ns - (zero_ns % 1000000000); // round down (shaves off up to 1 second)
    print_interval_ns = config.print_s * 1000000000ULL;
    print_next_ns = zero_ns;

    printf("create node\n");
    commkit::Node node;
    if (!node.init(prog)) {
        printf("error\n");
        exit(1);
    }

    commkit::Topic topic(TopicData::topic_name, TopicData::topic_type, sizeof(TopicData));

    printf("create subscriber\n");
    auto sub = node.createSubscriber(topic);
    if (sub == nullptr) {
        printf("error\n");
        exit(1);
    }
    sub->onPublisherConnected.connect(&onConnect);
    sub->onPublisherDisconnected.connect(&onDisconnect);
    sub->onMessage.connect(&onMessage);
    commkit::SubscriptionOpts sub_opts;
    sub_opts.reliable = config.reliable;
    sub_opts.history = config.history;
    if (!sub->init(sub_opts)) {
        printf("error\n");
        exit(1);
    }
    usleep(500000);

    printf("ready\n");

    while (true) {
        pause();
    }

    return 0;

} // main
