
#include <cstdint>
#include <iostream>
#include <set>
#include <thread>

#include <gtest/gtest.h>

#include "fastrtps/fastrtps_all.h"

#include "time_rtps.h"
#include "simple_stats.h"
#include "resources.h"
#include "topic_data_type.h"
#include "topic_data.h"

constexpr int domainId = 80;

// default is 65536; smaller causes drops at the socket layer when
// packets are sent very fast, exercising the reliable protocol
constexpr int socketBufSize = 8192;

Resources resources;

SimpleStats<std::int64_t> latency_stats;

inline int64_t toInt64(const SequenceNumber_t &s)
{
    return (int64_t(s.high) << 32) | s.low;
}

std::ostream &operator<<(std::ostream &os, const ReliabilityQosPolicyKind &r)
{
    if (r == BEST_EFFORT_RELIABILITY_QOS)
        os << "BEST_EFFORT";
    else if (r == RELIABLE_RELIABILITY_QOS)
        os << "RELIABLE";
    else {
        os << "reliability=" << unsigned(r);
    }
    return os;
}

TestTopicDataType topicDataType;

class TestPublisherListener : public PublisherListener
{
public:
    TestPublisherListener() : matched(0)
    {
        // std::cout << "TestPublisherListener" << std::endl;
    }
    virtual ~TestPublisherListener()
    {
        // std::cout << "~TestPublisherListener" << std::endl;
        matched = -1; // invalid
    }
    virtual void onPublicationMatched(Publisher *pub, rtps::MatchingInfo &info)
    {
        if (info.status == MATCHED_MATCHING) {
            matched++;
        } else {
            matched--;
        }
    }
    int matched;
};

class TestSubscriberListener : public SubscriberListener
{
public:
    TestSubscriberListener() : subscriber(nullptr), matched(0), lastSeq(SEQ_UNSET), received(0)
    {
        // std::cout << "TestSubscriberListener" << std::endl;
    }
    virtual ~TestSubscriberListener()
    {
        // std::cout << "~TestSubscriberListener" << std::endl;
        subscriber = nullptr;
        matched = -1;
        lastSeq = SEQ_UNSET;
        received = -1;
    }
    void onSubscriptionMatched(Subscriber *sub, rtps::MatchingInfo &info)
    {
        if (info.status == MATCHED_MATCHING) {
            subscriber = sub;
            matched++;
        } else {
            matched--;
        }
    }
    // void onNewDataMessage(Subscriber *sub) not used
    bool takeWithTimeout(TopicData &data, SampleInfo_t &info, TestClock::duration timeout)
    {
        TestClock::duration pollInterval = std::chrono::milliseconds(1);
        while (true) {
            if (subscriber == nullptr) {
                return false; // subscriber deleted?
            } else if (subscriber->takeNextData(&data, &info)) {
                int64_t seq = toInt64(info.sample_identity.sequence_number());
                sequenceNumbersSeen.insert(seq);
                return true;
            } else if (timeout <= TestClock::duration(0)) {
                return false; // timeout
            }
            std::this_thread::sleep_for(pollInterval);
            timeout -= pollInterval;
        }
    }
    void showSequenceNumbersSeen() const
    {
        cout << "sequence numbers seen: ";
        if (sequenceNumbersSeen.empty()) {
            cout << "none" << endl;
            return;
        }
        auto i = sequenceNumbersSeen.begin();
        int64_t s1 = *i;
        int64_t s2;
        cout << s1;
        int64_t seqLast = s1;
        i++;
        for (; i != sequenceNumbersSeen.end(); i++) {
            s2 = *i;
            if (s2 != (s1 + 1)) {
                if (s1 != seqLast)
                    cout << '-' << s1;
                cout << ' ' << s2;
                seqLast = s2;
            }
            s1 = s2;
        }
        if (s1 != seqLast)
            cout << '-' << s1;
        cout << endl;
    }
    static constexpr int64_t SEQ_UNSET = INT64_MAX;
    Subscriber *subscriber;
    int matched;
    int64_t lastSeq;
    int received;
    std::set<int64_t> sequenceNumbersSeen;
};

TEST(Test, History)
{
    /*
     * Create a pub and sub in different participants,
     * ensure we can send a message between them.
     */

    // eprosima::Log::setVerbosity(eprosima::VERB_ERROR);

    // histSmall is a history depth we can overflow quickly
    // histLarge is a history that we don't expect to overflow
    static constexpr int histSmall = 5;
    static constexpr int histLarge = 1000;

    for (ReliabilityQosPolicyKind qos : {BEST_EFFORT_RELIABILITY_QOS, RELIABLE_RELIABILITY_QOS}) {
        for (int pubHist : {histSmall, histLarge}) {
            for (int subHist : {histSmall, histLarge}) {

                std::cout << "qos=" << qos << " pubHist=" << pubHist << " subHist=" << subHist
                          << std::endl;

                // create publisher partition
                ParticipantAttributes pubPartAttr;
                pubPartAttr.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
                pubPartAttr.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
                pubPartAttr.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter =
                    true;
                pubPartAttr.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader =
                    true;
                pubPartAttr.rtps.builtin.domainId = domainId;
                pubPartAttr.rtps.sendSocketBufferSize = socketBufSize;
                pubPartAttr.rtps.listenSocketBufferSize = socketBufSize;
                pubPartAttr.rtps.setName("TestPublisher");
                Participant *pubPart = Domain::createParticipant(pubPartAttr);
                ASSERT_NE(pubPart, nullptr);

                // register publisher data type
                Domain::registerType(pubPart, &topicDataType);

                // create publisher listener
                TestPublisherListener pubList;

                // create publisher
                PublisherAttributes pubAttr;
                pubAttr.topic.topicKind = NO_KEY;
                pubAttr.topic.topicName = TopicData::topic_name;
                pubAttr.topic.topicDataType = TopicData::topic_type;
                pubAttr.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
                pubAttr.topic.historyQos.depth = pubHist;
                // Publisher's max_samples has to be at least 2 more than its
                // history depth; there is something in the way it allocates a
                // cache entry, then checks to see if history is full, where the
                // allocate can fail and the history clean is never attempted.
                // +1 is not enough; +2 seems to be okay.
                pubAttr.topic.resourceLimitsQos.max_samples = pubHist + 2;
                pubAttr.topic.resourceLimitsQos.allocated_samples = pubHist + 2;
                pubAttr.times.heartbeatPeriod = toTime(0.100);
                pubAttr.times.nackResponseDelay = toTime(0.010);
                pubAttr.qos.m_reliability.kind = qos;
                Publisher *pub = Domain::createPublisher(pubPart, pubAttr, &pubList);
                ASSERT_NE(pub, nullptr);

                // create subscriber partition
                ParticipantAttributes subPartAttr;
                subPartAttr.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
                subPartAttr.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
                subPartAttr.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter =
                    true;
                subPartAttr.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader =
                    true;
                subPartAttr.rtps.builtin.domainId = domainId;
                subPartAttr.rtps.sendSocketBufferSize = socketBufSize;
                subPartAttr.rtps.listenSocketBufferSize = socketBufSize;
                subPartAttr.rtps.setName("TestSubscriber");
                Participant *subPart = Domain::createParticipant(subPartAttr);
                ASSERT_NE(subPart, nullptr);

                // register subscriber data type
                Domain::registerType(subPart, &topicDataType);

                // create subscriber listener
                TestSubscriberListener subList;

                // create subscriber
                SubscriberAttributes subAttr;
                subAttr.topic.topicKind = NO_KEY;
                subAttr.topic.topicName = TopicData::topic_name;
                subAttr.topic.topicDataType = TopicData::topic_type;
                subAttr.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
                subAttr.topic.historyQos.depth = subHist;
                subAttr.topic.resourceLimitsQos.max_samples = subHist + 2;
                subAttr.topic.resourceLimitsQos.allocated_samples = subHist + 2;
                subAttr.times.heartbeatResponseDelay = toTime(0.010);
                subAttr.qos.m_reliability.kind = qos;
                Subscriber *sub = Domain::createSubscriber(subPart, subAttr, &subList);
                ASSERT_NE(sub, nullptr);

                // should match very quickly - typical is on the third loop (after 2 sleeps)
                int matchLoops;
                for (matchLoops = 0; matchLoops < 100; matchLoops++) {
                    if (pubList.matched == 1 && subList.matched == 1)
                        break;
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                ASSERT_EQ(pubList.matched, 1);
                ASSERT_EQ(subList.matched, 1);
                // typical here is "matched in 2 msec" (occasionally 1 msec)
                // std::cout << "matched in " << matchLoops << " msec" << std::endl;

                // Fast-RTPS might have a bug where if it generates a sequence
                // number set that spans more than 256 sequence numbers, it
                // fails an assert instead of doing whatever RTPS says it
                // should do. With the small socket buffers, about 100 messages
                // is enough to (usually) trigger resends on reliable streams.
                // With release builds and msgCount=100, the acknack asks for
                // ~40..50 to be resent. This shouldn't be more than 255 in
                // order to avoid the assert.
                int msgCount = 250;

                // subTimeout should be long enough for pub to send another heartbeat
                // and sub to request more messages
                TestClock::duration subTimeout = std::chrono::milliseconds(200);

                // evenly spaced, should not miss any
                // cout << "smooth..." << endl;
                subList.sequenceNumbersSeen.clear();
                TestClock::duration msgInterval = std::chrono::milliseconds(1);
                TestClock::time_point msgTime = TestClock::now();
                msgTime += msgInterval;
                for (int i = 0; i < msgCount; i++) {
                    std::this_thread::sleep_until(msgTime);
                    TopicData pubData;
                    EXPECT_TRUE(pub->write(&pubData));
                    TopicData subData;
                    SampleInfo_t sampleInfo;
                    EXPECT_TRUE(subList.takeWithTimeout(subData, sampleInfo, subTimeout));
                    msgTime += msgInterval;
                }

                // burst-send to overrun history - might drop some, but should recover
                // cout << "bursty..." << endl;
                int pubCount = 0;
                for (int i = 0; i < msgCount; i++) {
                    TopicData pubData;
                    if (pub->write(&pubData))
                        pubCount++;
                }
                // should always have sent them all
                EXPECT_GE(pubCount, msgCount);
                // std::cout << "pubCount=" << pubCount << std::endl;
                // see what subscriber got
                // XXX is it possible that once we start pulling messages from
                // the subscriber, it might ask the publisher for more?
                subList.sequenceNumbersSeen.clear();
                int subCount = 0;
                TopicData subData;
                SampleInfo_t sampleInfo;
                while (subList.takeWithTimeout(subData, sampleInfo, subTimeout)) {
                    subCount++;
                }
                // should always have received some
                EXPECT_GT(subCount, 0);
                // How many depends on qos and history depth. The test
                // at this point just makes sure we got _some_; that the
                // system continues working in the face of overruns.
                // std::cout << "subCount=" << subCount << std::endl;
                // subList.showSequenceNumbersSeen();
                if (qos == RELIABLE_RELIABILITY_QOS && msgCount < subHist && msgCount < pubHist) {
                    // no excuse for not getting them all
                    EXPECT_EQ(subCount, msgCount);
                }

                // delete subscriber
                ASSERT_TRUE(Domain::removeSubscriber(sub));
                // delete subscriber participant
                ASSERT_TRUE(Domain::removeParticipant(subPart));

                // delete publisher
                ASSERT_TRUE(Domain::removePublisher(pub));
                // delete publisher participant
                ASSERT_TRUE(Domain::removeParticipant(pubPart));

            } // for (subHist...)
        }     // for (pubHist...)
    }         // for (qos...)
}
