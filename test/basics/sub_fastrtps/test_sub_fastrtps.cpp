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

#include <fastrtps/Domain.h>
#include <fastrtps/TopicDataType.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/log/Log.h>

#include "time_util.h"

#include "topic_data.h"
#include "test_config.h"
#include "resources.h"

using namespace eprosima::fastrtps;

static const char *prog = "test_sub_fastrtps";

uint64_t zero_ns;
uint64_t print_interval_ns;
uint64_t print_next_ns;

static volatile int msg_count = -1;

static Resources resources;

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

class TestSubscriberListener : public SubscriberListener
{
public:
    TestSubscriberListener() : _matched(0), _last_seq(INT64_MAX)
    {
    }

    void onSubscriptionMatched(Subscriber *sub, rtps::MatchingInfo &info)
    {
        if (info.status == MATCHED_MATCHING) {
            _matched++;
            printf("matched\n");
        } else {
            _matched--;
            printf("unmatched\n");
        }
    }

    void onNewDataMessage(Subscriber *sub)
    {
        TopicData topic_data;
        SampleInfo_t sample_info;
        while (msg_count != 0 && sub->takeNextData((void *)&topic_data, &sample_info)) {

            if (msg_count > 0)
                msg_count--;

            if (sample_info.sampleKind != ALIVE) {
                continue;
            }

            // look for and print gaps in sequence number
            int64_t sequence = toInt64(sample_info.sample_identity.sequence_number());
            if (_last_seq != INT64_MAX && (sequence - _last_seq) > 1) {
                printf("gap: %u\n", unsigned(sequence - _last_seq - 1));
            }
            _last_seq = sequence;

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

    int64_t get_last_seq() const { return _last_seq; }

private:
    int _matched;
    int64_t _last_seq;
};

static TestTopicDataType topic_data_type;

static TestSubscriberListener test_subscriber_listener;

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

    msg_count = config.count;

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

    printf("create subscriber\n");
    SubscriberAttributes sub_attr;
    sub_attr.topic.topicKind = NO_KEY;
    sub_attr.topic.topicName = TopicData::topic_name;
    sub_attr.topic.topicDataType = TopicData::topic_type;
    sub_attr.times.heartbeatResponseDelay.seconds = 0;
    sub_attr.times.heartbeatResponseDelay.fraction = 4294967 * 50; // ~50 millis;
    sub_attr.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    sub_attr.topic.historyQos.depth = config.history;
    sub_attr.topic.resourceLimitsQos.max_samples = 2 * config.history;
    sub_attr.topic.resourceLimitsQos.allocated_samples = 2 * config.history;
    sub_attr.qos.m_reliability.kind =
        config.reliable ? RELIABLE_RELIABILITY_QOS : BEST_EFFORT_RELIABILITY_QOS;
    Subscriber *sub = Domain::createSubscriber(part, sub_attr, &test_subscriber_listener);
    if (sub == nullptr) {
        printf("error\n");
        exit(1);
    }

    printf("ready\n");

    while (msg_count != 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    Domain::removeSubscriber(sub);
    Domain::removeParticipant(part);

    return 0;

} // main
