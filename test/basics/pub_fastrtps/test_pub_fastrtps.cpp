#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fastrtps/Domain.h>
#include <fastrtps/TopicDataType.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/log/Log.h>

#include "time_util.h"

#include "topic_data.h"
#include "test_config.h"
#include "resources.h"

using namespace eprosima::fastrtps;

static const char *prog = "test_pub_fastrtps";

class TestTopicDataType : public TopicDataType
{

public:
    TestTopicDataType()
    {
        setName(TopicData::topic_type);
        m_typeSize = sizeof(TopicData);
        m_isGetKeyDefined = false;
    }

    bool serialize(void *data, SerializedPayload_t *payload)
    {
        memcpy(payload->data, data, sizeof(TopicData));
        payload->length = sizeof(TopicData);
        return true;
    }

    bool deserialize(SerializedPayload_t *payload, void *data)
    {
        memcpy(data, payload->data, sizeof(TopicData));
        return true;
    }

    void *createData()
    {
        return new TopicData();
    }

    void deleteData(void *data)
    {
        delete (TopicData *)data;
    }
};

class TestPublisherListener : public PublisherListener
{
public:
    TestPublisherListener() : _matched(0)
    {
    }

    virtual void onPublicationMatched(Publisher *pub, rtps::MatchingInfo &info)
    {
        if (info.status == MATCHED_MATCHING) {
            _matched++;
            printf("matched\n");
        } else {
            _matched--;
            printf("unmatched\n");
        }
    }

private:
    int _matched;
};

static TestTopicDataType topic_data_type;

static TestPublisherListener test_publisher_listener;

int main(int argc, char *argv[])
{
    printf("built %s %s\n", __DATE__, __TIME__);

    TestConfig::Config config;
    if (!TestConfig::parse_args(argc, argv, config)) {
        TestConfig::usage(prog);
    }

    eprosima::Log::setVerbosity(eprosima::VERB_ERROR);

    Resources resources;

    printf("create participant\n");
    ParticipantAttributes part_attr;
    part_attr.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
    part_attr.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
    part_attr.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    part_attr.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    part_attr.rtps.builtin.domainId = 80;
    part_attr.rtps.builtin.leaseDuration = c_TimeInfinite;
    part_attr.rtps.sendSocketBufferSize = 8712;
    part_attr.rtps.listenSocketBufferSize = 17424;
    part_attr.rtps.setName(prog);
    Participant *part = Domain::createParticipant(part_attr);
    if (part == nullptr) {
        printf("error\n");
        exit(1);
    }

    Domain::registerType(part, &topic_data_type);

    printf("create publisher\n");
    PublisherAttributes pub_attr;
    pub_attr.topic.topicKind = NO_KEY;
    pub_attr.topic.topicName = TopicData::topic_name;
    pub_attr.topic.topicDataType = TopicData::topic_type;
    pub_attr.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    pub_attr.topic.historyQos.depth = config.history;
    pub_attr.topic.resourceLimitsQos.max_samples = 2 * config.history;
    pub_attr.topic.resourceLimitsQos.allocated_samples = 2 * config.history;
    pub_attr.times.heartbeatPeriod.seconds = 0;
    pub_attr.times.heartbeatPeriod.fraction = 4294967 * 100; // ~100 millis
    pub_attr.qos.m_reliability.kind =
        config.reliable ? RELIABLE_RELIABILITY_QOS : BEST_EFFORT_RELIABILITY_QOS;
    Publisher *pub = Domain::createPublisher(part, pub_attr, &test_publisher_listener);
    if (pub == nullptr) {
        printf("error\n");
        exit(1);
    }

    // zero time is next one-second boundary that is least 0.1 sec away
    // 1.1 sec from now
    uint64_t zero_ns = clock_gettime_ns(CLOCK_MONOTONIC) + 1100000000ULL;
    // round down (shaves up to 1 second)
    zero_ns = zero_ns - (zero_ns % 1000000000ULL);

    uint64_t pub_interval_ns = 1000000000ULL / config.rate;
    uint64_t pub_next_ns = zero_ns;

    uint64_t print_interval_ns = config.print_s * 1000000000ULL;
    uint64_t print_next_ns = zero_ns;

    uint32_t sequence = 0;

    while (config.count != 0) {

        uint64_t now_ns = clock_gettime_ns(CLOCK_MONOTONIC);
        uint32_t delay_us = 0;
        if (pub_next_ns > now_ns) {
            delay_us = (pub_next_ns - now_ns) / 1000;
        }
        usleep(delay_us);

        // time at this moment is intended to be pub_next_ns;
        // base calculations on that for consistency

        TopicData topic_data;
        memset(&topic_data, 0, sizeof(TopicData));
        pub->write((void *)&topic_data);

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

        if (config.count > 0)
            config.count--;

    } // while (config.count != 0)

    Domain::removePublisher(pub);
    Domain::removeParticipant(part);

    return 0;

} // main
