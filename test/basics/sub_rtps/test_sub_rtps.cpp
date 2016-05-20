#include <cstdio>
#include <unistd.h>

#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/qos/ReaderQos.h>
#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/rtps/attributes/ReaderAttributes.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/utils/RTPSLog.h>

#include "time_util.h"

#include "topic_data.h"
#include "test_config.h"
#include "resources.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

static const char *prog = "test_sub_rtps";

uint64_t zero_ns;
uint64_t printInterval_ns;
uint64_t printNext_ns;

static Resources resources;

// sequence number in last packet received
static uint32_t lastSeq = UINT32_MAX;

class TestReaderListener : public ReaderListener
{
public:
    TestReaderListener() : _matched(0)
    {
    }

    ~TestReaderListener()
    {
    }

    virtual void onReaderMatched(RTPSReader *r, MatchingInfo &info)
    {
        if (info.status == MATCHED_MATCHING) {
            _matched++;
            printf("matched: %d writers\n", _matched);
        } else {
            _matched--;
            printf("unmatched: %d writers\n", _matched);
        }
    }

    virtual void onNewCacheChangeAdded(RTPSReader *r, const CacheChange_t *const cc)
    {
        if (cc->kind != ALIVE) {
            printf("message kind != ALIVE\n");
        } else if (cc->serializedPayload.length != sizeof(TopicData)) {
            // arm-oe-linux-gnueabi-g++ 4.8.1 expects sizeof() to be %u
            // g++ 4.8.4 expects sizeof() to be %lu
            printf("message wrong length (expected %u, got %u)\n", (unsigned)sizeof(TopicData),
                   cc->serializedPayload.length);
        } else {

            TopicData topicData;
            memcpy(&topicData, cc->serializedPayload.data, sizeof(topicData));

            // look for and print gaps in sequence number
            uint32_t sequence = topicData.sequence;
            if (lastSeq != UINT32_MAX && (sequence - lastSeq) > 1) {
                printf("gap: %u %50c\n", sequence - lastSeq - 1, ' ');
            }
            lastSeq = sequence;

            uint64_t now_ns = clock_gettime_ns(CLOCK_MONOTONIC);
            if (now_ns >= printNext_ns) {
                resources.sample();
                double cpu = resources.cpuLoad();
                uint64_t msec = (now_ns - zero_ns + 500000) / 1000000;
                printf("%-18s %6u.%03u: %6u %5.1f%%\n", prog, (unsigned)(msec / 1000),
                       (unsigned)(msec % 1000), sequence, cpu * 100);
                printNext_ns += printInterval_ns;
            }
        }
        r->getHistory()->remove_change(const_cast<CacheChange_t *>(cc));
    }

private:
    int _matched;
};

int main(int argc, char *argv[])
{
    printf("built %s %s\n", __DATE__, __TIME__);

    TestConfig::Config config;
    if (!TestConfig::parseArgs(argc, argv, config)) {
        TestConfig::usage(prog);
    }

    eprosima::Log::setVerbosity(eprosima::VERB_ERROR);

    // zero time is next one-second boundary that is least 0.1 sec away
    zero_ns = clock_gettime_ns(CLOCK_MONOTONIC) + 1100000000; // 1.1 sec from now
    zero_ns = zero_ns - (zero_ns % 1000000000); // round down (shaves off up to 1 second)
    printInterval_ns = config.print_s * 1000000000ULL;
    printNext_ns = zero_ns;

    printf("create rtps participant\n");
    RTPSParticipantAttributes partAttr;
    partAttr.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
    partAttr.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
    partAttr.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    partAttr.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    partAttr.builtin.domainId = 80;
    partAttr.builtin.leaseDuration = c_TimeInfinite;
    partAttr.sendSocketBufferSize = 8712;
    partAttr.listenSocketBufferSize = 17424;
    partAttr.setName("test_participant");
    RTPSParticipant *part = RTPSDomain::createParticipant(partAttr);
    if (part == nullptr) {
        printf("error\n");
        exit(1);
    }

    printf("create rtps reader history\n");
    HistoryAttributes histAttr;
    histAttr.payloadMaxSize = sizeof(TopicData);
    // XXX config.history
    ReaderHistory *readerHist = new ReaderHistory(histAttr);
    if (readerHist == nullptr) {
        printf("error\n");
        exit(1);
    }

    printf("create rtps reader\n");
    TestReaderListener readerListener;
    ReaderAttributes readerAttr;
    readerAttr.times.heartbeatResponseDelay.seconds = 0;
    readerAttr.times.heartbeatResponseDelay.fraction = 4294967 * 50; // ~50 millis;
    readerAttr.endpoint.reliabilityKind = config.reliable ? RELIABLE : BEST_EFFORT;
    RTPSReader *reader =
        RTPSDomain::createRTPSReader(part, readerAttr, readerHist, &readerListener);
    if (reader == nullptr) {
        printf("error\n");
        exit(1);
    }

    printf("register rtps reader\n");
    TopicAttributes topicAttr;
    topicAttr.topicKind = NO_KEY;
    topicAttr.topicName = TopicData::topicName;
    topicAttr.topicDataType = TopicData::topicType;
    ReaderQos readerQos;
    if (!part->registerReader(reader, topicAttr, readerQos)) {
        printf("error\n");
        exit(1);
    }

    printf("ready\n");

    while (true) {
        pause();
    }

    return 0;

} // main
