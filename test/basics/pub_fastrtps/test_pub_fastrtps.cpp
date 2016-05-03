#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

#include <fastrtps/Domain.h>
#include <fastrtps/TopicDataType.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/log/Log.h>

#include "topic_data.h"
#include "test_config.h"
#include "resources.h"

using namespace eprosima::fastrtps;

static const char *prog = "test_pub_fastrtps";

inline double toDouble(std::chrono::steady_clock::duration d)
{
    return std::chrono::duration_cast<std::chrono::duration<double>>(d).count();
}

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

    std::chrono::steady_clock::time_point time_zero = std::chrono::steady_clock::now();

    std::chrono::steady_clock::time_point pub_next = time_zero;
    std::chrono::steady_clock::duration pub_interval =
        std::chrono::steady_clock::duration(std::chrono::nanoseconds(1000000000ull / config.rate));

    std::chrono::steady_clock::time_point print_next = time_zero;
    std::chrono::steady_clock::duration print_interval =
        std::chrono::steady_clock::duration(std::chrono::seconds(config.print_s));

    uint32_t sequence = 0;

    while (config.count != 0) {

        std::this_thread::sleep_until(pub_next);

        TopicData topic_data;
        memset(&topic_data, 0, sizeof(TopicData));
        pub->write((void *)&topic_data);

        if (pub_next >= print_next) {
            resources.sample();
            double cpu = resources.cpu_load();
            std::ios::fmtflags f(std::cout.flags()); // save state
            std::cout << std::setw(18) << std::left << prog << std::right << " " << std::setw(10)
                      << std::fixed << std::setprecision(3) << toDouble(pub_next - time_zero)
                      << ": " << std::setw(6) << sequence << " " << std::setw(5) << std::fixed
                      << std::setprecision(1) << cpu * 100.0 << "%" << std::endl;
            std::cout.flags(f); // restore state
            print_next += print_interval;
        }

        sequence++;
        pub_next += pub_interval;

        if (config.count > 0)
            config.count--;

    } // while (config.count != 0)

    Domain::removePublisher(pub);
    Domain::removeParticipant(part);

    return 0;

} // main
