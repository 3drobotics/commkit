#include <errno.h>
#include <getopt.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <thread>

#include <fastrtps/Domain.h>
#include <fastrtps/TopicDataType.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/log/Log.h>

#include "resources.h"
#include "simple_stats.h"
#include "test_config.h"
#include "time_rtps.h"
#include "topic_data.h"

using namespace eprosima::fastrtps;

static const char *prog = "test_sub_fastrtps";

static test_clock::time_point timeZero;
static test_clock::time_point printNext;
static test_clock::duration printInterval;

static volatile int msgCount = -1;

static Resources resources;

static SimpleStats<std::int64_t> latency_stats;

inline int64_t toInt64(const SequenceNumber_t &s)
{
    return (int64_t(s.high) << 32) | s.low;
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

class TestSubscriberListener : public SubscriberListener
{
public:
    TestSubscriberListener() : _matched(0), _lastSeq(INT64_MAX)
    {
    }

    void onSubscriptionMatched(Subscriber *sub, rtps::MatchingInfo &info)
    {
        if (info.status == MATCHED_MATCHING) {
            _matched++;
            std::cout << "matched" << std::endl;
        } else {
            _matched--;
            std::cout << "unmatched" << std::endl;
        }
    }

    void onNewDataMessage(Subscriber *sub)
    {
        TopicData topicData;
        SampleInfo_t sampleInfo;
        while (msgCount != 0 && sub->takeNextData((void *)&topicData, &sampleInfo)) {

            test_clock::time_point now = test_clock::now();

            if (msgCount > 0)
                msgCount--;

            if (sampleInfo.sampleKind != ALIVE) {
                continue;
            }

            // look for and print gaps in sequence number
            int64_t sequence = toInt64(sampleInfo.sample_identity.sequence_number());
            if (_lastSeq != INT64_MAX && (sequence - _lastSeq) > 1) {
                std::cout << "gap: " << unsigned(sequence - _lastSeq - 1) << std::endl;
            }
            _lastSeq = sequence;

            // look for and print long latencies
            test_clock::duration latency = now - toTimePoint(sampleInfo.sourceTimestamp);
            latency_stats.accumulate(toInt64(latency));
            if (latency > std::chrono::milliseconds(100)) {
                std::chrono::milliseconds msec =
                    std::chrono::duration_cast<std::chrono::milliseconds>(latency);
                std::cout << unsigned(sequence) << " delayed " << msec.count() << " msec"
                          << std::endl;
            }

            if (now >= printNext) {
                resources.sample();
                double cpu = resources.cpu_load();
                std::ios::fmtflags f(std::cout.flags()); // save state
                std::cout << std::setw(18) << std::left << prog << std::right << " "
                          << std::setw(10) << std::fixed << std::setprecision(3)
                          << toDouble(now - timeZero) << ": " << std::setw(6) << sequence << " "
                          // min max avg in milliseconds
                          << std::setw(6) << std::fixed << std::setprecision(2)
                          << latency_stats.min() / 1000000.0 << " " << std::setw(6) << std::fixed
                          << std::setprecision(2) << latency_stats.max() / 1000000.0 << " "
                          << std::setw(6) << std::fixed << std::setprecision(2)
                          << latency_stats.average() / 1000000.0 << " " << std::setw(5)
                          << std::fixed << std::setprecision(1) << cpu * 100.0 << "%" << std::endl;
                std::cout.flags(f); // restore state
                latency_stats.reset();
                printNext += printInterval;
            }
        }
    }

    int64_t getLastSeq() const
    {
        return _lastSeq;
    }

private:
    int _matched;
    int64_t _lastSeq;
};

static TestTopicDataType topicDataType;

static TestSubscriberListener testSubscriberListener;

int main(int argc, char *argv[])
{
    std::cout << "built " << __DATE__ << " " << __TIME__ << std::endl;

    TestConfig::Config config;
    if (!TestConfig::parse_args(argc, argv, config)) {
        TestConfig::usage(prog);
    }

    eprosima::Log::setVerbosity(eprosima::VERB_ERROR);

    printInterval = test_clock::duration(std::chrono::seconds(config.print_s));
    timeZero = test_clock::now();
    printNext = timeZero;

    msgCount = config.count;

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

    std::cout << "create subscriber" << std::endl;
    SubscriberAttributes subAttr;
    subAttr.topic.topicKind = NO_KEY;
    subAttr.topic.topicName = TopicData::topic_name;
    subAttr.topic.topicDataType = TopicData::topic_type;
    subAttr.times.heartbeatResponseDelay = toTime(0.050);
    subAttr.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    subAttr.topic.historyQos.depth = config.history;
    subAttr.topic.resourceLimitsQos.max_samples = 2 * config.history;
    subAttr.topic.resourceLimitsQos.allocated_samples = 2 * config.history;
    subAttr.qos.m_reliability.kind =
        config.reliable ? RELIABLE_RELIABILITY_QOS : BEST_EFFORT_RELIABILITY_QOS;
    Subscriber *sub = Domain::createSubscriber(part, subAttr, &testSubscriberListener);
    if (sub == nullptr) {
        std::cerr << "error" << std::endl;
        exit(1);
    }

    std::cout << "ready" << std::endl;

    int64_t lastSeqSave = INT64_MAX;
    auto seqChange = test_clock::now();
    while (msgCount != 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto now = test_clock::now();
        int64_t seq = testSubscriberListener.getLastSeq();
        if (lastSeqSave != seq) {
            lastSeqSave = seq;
            seqChange = now;
        } else if ((now - seqChange) > std::chrono::seconds(2)) {
            // this print is the point of lastSeqSave etc.
            cout << "sequence " << seq << " not changing" << endl;
            seqChange = now;
        } else {
            // seq did not change, but not printing yet
        }
    }

    Domain::removeSubscriber(sub);
    Domain::removeParticipant(part);

    return 0;

} // main
