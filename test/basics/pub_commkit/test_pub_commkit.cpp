#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fastrtps/log/Log.h"

#include <commkit/node.h>
#include <commkit/publisher.h>

#include "time_util.h"
#include "wire_util.h"

#include "topic_data.h"
#include "test_config.h"
#include "resources.h"

static const char *prog = "test_pub_commkit";

static void onConnect(const commkit::PublisherPtr pub)
{
    printf("subscriber connected (%u)\n", pub->matchedSubscribers());
}

static void onDisconnect(const commkit::PublisherPtr pub)
{
    printf("subscriber disconnected (%u)\n", pub->matchedSubscribers());
}

int main(int argc, char *argv[])
{
    printf("built %s %s\n", __DATE__, __TIME__);

    TestConfig::Config config;
    if (!TestConfig::parse_args(argc, argv, config)) {
        TestConfig::usage(prog);
    }

    eprosima::Log::setVerbosity(eprosima::VERB_ERROR);

    Resources resources;

    printf("create node\n");
    commkit::Node node;
    if (!node.init(prog)) {
        printf("error\n");
        exit(1);
    }

    commkit::Topic topic(TopicData::topic_name, TopicData::topic_type, sizeof(TopicData));

    printf("create publisher\n");
    auto pub = node.createPublisher(topic);
    if (pub == nullptr) {
        printf("error\n");
        exit(1);
    }
    pub->onSubscriberConnected.connect(&onConnect);
    pub->onSubscriberDisconnected.connect(&onDisconnect);
    commkit::PublicationOpts pub_opts;
    pub_opts.reliable = config.reliable;
    pub_opts.history = config.history;
    if (!pub->init(pub_opts)) {
        printf("error\n");
        exit(1);
    }

    // zero time is next one-second boundary that is least 0.1 sec away
    // 1.1 sec from now
    uint64_t zero_ns = clock_gettime_ns(CLOCK_MONOTONIC) + 1100000000ULL;
    // round down (shaves off up to 1 second)
    zero_ns = zero_ns - (zero_ns % 1000000000ULL);

    uint64_t pub_interval_ns = 1000000000ULL / config.rate;
    uint64_t pub_next_ns = zero_ns;

    uint64_t print_interval_ns = config.print_s * 1000000000ULL;
    uint64_t print_next_ns = zero_ns;

    uint32_t sequence = 0;

    while (true) {
        // sleep until pub_next_ns
        uint64_t now_ns = clock_gettime_ns(CLOCK_MONOTONIC);
        uint32_t delay_us = 0;
        if (pub_next_ns > now_ns) {
            delay_us = (pub_next_ns - now_ns) / 1000;
        }
        usleep(delay_us);

        // time at this moment is intended to be pub_next_ns;
        // base calculations on that for consistency

        uint8_t *pub_buf;
        if (pub->reserve(&pub_buf, sizeof(TopicData))) {
            TopicData topic_data;
            memset(&topic_data, 0, sizeof(TopicData));
            topic_data.timestamp_ns = pub_next_ns;
            topic_data.sequence = sequence;
            memcpy(pub_buf, &topic_data, sizeof(TopicData));
            pub->publishReserved(pub_buf, sizeof(TopicData));
        } else {
            printf("can't reserve publish buffer (TopicData)\n");
        }

        if (pub_next_ns >= print_next_ns) {
            resources.sample();
            double cpu = resources.cpu_load();
            uint64_t msec = (pub_next_ns - zero_ns + 500000) / 1000000;
            printf("%-18s %6u.%03u: %6u %5.1f%%\n", prog, (unsigned)(msec / 1000),
                   (unsigned)(msec % 1000), sequence, cpu * 100);
            print_next_ns += print_interval_ns;
        }

        sequence++;
        pub_next_ns += pub_interval_ns;
    }

    return 0;

} // main
