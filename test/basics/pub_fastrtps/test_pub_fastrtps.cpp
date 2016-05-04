#include <errno.h>
#include <stdint.h>
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

#include "resources.h"
#include "test_config.h"
#include "time_rtps.h"
#include "topic_data.h"

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
            std::cout << "matched" << std::endl;
        } else {
            _matched--;
            std::cout << "unmatched" << std::endl;
        }
    }

private:
    int _matched;
};

static TestTopicDataType topicDataType;

static TestPublisherListener testPublisherListener;

int main(int argc, char *argv[])
{
    std::cout << "built " << __DATE__ << " " << __TIME__ << std::endl;

    TestConfig::Config config;
    if (!TestConfig::parse_args(argc, argv, config)) {
        TestConfig::usage(prog);
    }

    eprosima::Log::setVerbosity(eprosima::VERB_ERROR);

    Resources resources;

    std::cout << "create participant" << std::endl;
    ParticipantAttributes partAttr;
    partAttr.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
    partAttr.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
    partAttr.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    partAttr.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    partAttr.rtps.builtin.domainId = 80;
    partAttr.rtps.builtin.leaseDuration = toTime(config.lease_s);
    partAttr.rtps.builtin.leaseDuration_announcementperiod = toTime(config.renew_s);
    partAttr.rtps.setName(prog);
    Participant *part = Domain::createParticipant(partAttr);
    if (part == nullptr) {
        std::cerr << "error" << std::endl;
        exit(1);
    }

    Domain::registerType(part, &topicDataType);

    std::cout << "create publisher" << std::endl;
    PublisherAttributes pubAttr;
    pubAttr.topic.topicKind = NO_KEY;
    pubAttr.topic.topicName = TopicData::topic_name;
    pubAttr.topic.topicDataType = TopicData::topic_type;
    pubAttr.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    pubAttr.topic.historyQos.depth = config.history;
    pubAttr.topic.resourceLimitsQos.max_samples = 2 * config.history;
    pubAttr.topic.resourceLimitsQos.allocated_samples = 2 * config.history;
    pubAttr.times.heartbeatPeriod = toTime(0.100);
    pubAttr.qos.m_reliability.kind =
        config.reliable ? RELIABLE_RELIABILITY_QOS : BEST_EFFORT_RELIABILITY_QOS;
    Publisher *pub = Domain::createPublisher(part, pubAttr, &testPublisherListener);
    if (pub == nullptr) {
        std::cerr << "error" << std::endl;
        exit(1);
    }

    std::chrono::steady_clock::time_point timeZero = std::chrono::steady_clock::now();

    std::chrono::steady_clock::time_point pubNext = timeZero;
    std::chrono::steady_clock::duration pubInterval =
        std::chrono::steady_clock::duration(std::chrono::nanoseconds(1000000000ull / config.rate));

    std::chrono::steady_clock::time_point printNext = timeZero;
    std::chrono::steady_clock::duration printInterval =
        std::chrono::steady_clock::duration(std::chrono::seconds(config.print_s));

    std::cout << "ready" << std::endl;

    uint32_t sequence = 0;

    while (config.count != 0) {

        std::this_thread::sleep_until(pubNext);

        TopicData topicData;
        memset(&topicData, 0, sizeof(TopicData));
        pub->write((void *)&topicData);

        if (pubNext >= printNext) {
            resources.sample();
            double cpu = resources.cpu_load();
            std::ios::fmtflags f(std::cout.flags()); // save state
            std::cout << std::setw(18) << std::left << prog << std::right << " " << std::setw(10)
                      << std::fixed << std::setprecision(3) << toDouble(pubNext - timeZero) << ": "
                      << std::setw(6) << sequence << " " << std::setw(5) << std::fixed
                      << std::setprecision(1) << cpu * 100.0 << "%" << std::endl;
            std::cout.flags(f); // restore state
            printNext += printInterval;
        }

        sequence++;
        pubNext += pubInterval;

        if (config.count > 0)
            config.count--;

    } // while (config.count != 0)

    Domain::removePublisher(pub);
    Domain::removeParticipant(part);

    return 0;

} // main
