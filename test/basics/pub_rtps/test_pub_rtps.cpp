#include <cstdio>
#include <unistd.h>

#include "fastrtps/attributes/TopicAttributes.h"
#include "fastrtps/qos/WriterQos.h"
#include "fastrtps/rtps/RTPSDomain.h"
#include "fastrtps/rtps/attributes/RTPSParticipantAttributes.h"
#include "fastrtps/rtps/attributes/HistoryAttributes.h"
#include "fastrtps/rtps/attributes/WriterAttributes.h"
#include "fastrtps/rtps/participant/RTPSParticipant.h"
#include "fastrtps/rtps/history/WriterHistory.h"
#include "fastrtps/rtps/writer/RTPSWriter.h"
#include "fastrtps/rtps/writer/WriterListener.h"
#include "fastrtps/utils/RTPSLog.h"

#include "time_util.h"

#include "topic_data.h"
#include "test_config.h"
#include "resources.h"

using namespace eprosima::fastrtps::rtps;

static const char *prog = "test_pub_rtps";

class TestWriterListener : public WriterListener
{
public:
    TestWriterListener() : _matched(0)
    {
    }

    ~TestWriterListener()
    {
    }

    void onWriterMatched(RTPSWriter *w, MatchingInfo &info)
    {
        if (info.status == MATCHED_MATCHING) {
            _matched++;
            printf("matched: %d readers\n", _matched);
        } else {
            _matched--;
            printf("unmatched: %d readers\n", _matched);
        }
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

    Resources resources;

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
    partAttr.setName(prog);
    RTPSParticipant *part = RTPSDomain::createParticipant(partAttr);
    if (part == nullptr) {
        printf("error\n");
        exit(1);
    }

    printf("create rtps writer history\n");
    HistoryAttributes histAttr;
    histAttr.payloadMaxSize = sizeof(TopicData);
    // XXX config.history
    WriterHistory *writerHist = new WriterHistory(histAttr);
    if (writerHist == nullptr) {
        printf("error\n");
        exit(1);
    }

    printf("create rtps writer\n");
    TestWriterListener writerListener;
    WriterAttributes writerAttr;
    writerAttr.times.heartbeatPeriod.seconds = 0;
    writerAttr.times.heartbeatPeriod.fraction = 4294967 * 100; // ~100 millis
    writerAttr.endpoint.reliabilityKind = config.reliable ? RELIABLE : BEST_EFFORT;
    RTPSWriter *writer =
        RTPSDomain::createRTPSWriter(part, writerAttr, writerHist, &writerListener);
    if (writer == nullptr) {
        printf("error\n");
        exit(1);
    }

    printf("register rtps writer\n");
    TopicAttributes topicAttr;
    topicAttr.topicKind = NO_KEY;
    topicAttr.topicName = TopicData::topicName;
    topicAttr.topicDataType = TopicData::topicType;
    WriterQos writerQos;
    if (!part->registerWriter(writer, topicAttr, writerQos)) {
        printf("error\n");
        exit(1);
    }

    // zero time is next one-second boundary that is least 0.1 sec away
    // 1.1 sec from now
    uint64_t zero_ns = clock_gettime_ns(CLOCK_MONOTONIC) + 1100000000ULL;
    // round down (shaves off up to 1 second)
    zero_ns = zero_ns - (zero_ns % 1000000000ULL);

    uint64_t pubInterval_ns = 1000000000ULL / config.rate;
    uint64_t pubNext_ns = zero_ns;

    uint64_t printInterval_ns = config.print_s * 1000000000ULL;
    uint64_t printNext_ns = zero_ns;

    uint32_t sequence = 0;

    while (true) {
        // sleep until pubNext_ns
        uint64_t now_ns = clock_gettime_ns(CLOCK_MONOTONIC);
        uint32_t delay_us = 0;
        if (pubNext_ns > now_ns) {
            delay_us = (pubNext_ns - now_ns) / 1000;
        }
        usleep(delay_us);

        // time at this moment is intended to be pubNext_ns;
        // base calculations on that for consistency

        CacheChange_t *cacheChange = writer->new_change(ALIVE);

        TopicData topicData;
        memset(&topicData, 0, sizeof(TopicData));
        topicData.timestamp_ns = pubNext_ns;
        topicData.sequence = sequence;
        memcpy(cacheChange->serializedPayload.data, &topicData, sizeof(TopicData));
        cacheChange->serializedPayload.length = sizeof(TopicData);

        writerHist->add_change(cacheChange);

        if (pubNext_ns >= printNext_ns) {
            resources.sample();
            double cpu = resources.cpuLoad();
            uint64_t msec = (pubNext_ns - zero_ns + 500000) / 1000000;
            printf("%-18s %6u.%03u: %6u %5.1f%%\n", prog, (unsigned)(msec / 1000),
                   (unsigned)(msec % 1000), sequence, cpu * 100);
            printNext_ns += printInterval_ns;
        }

        sequence++;
        pubNext_ns += pubInterval_ns;
    }

    // XXX unregister writer?

    printf("remove rtps writer\n");
    // XXX does this delete it?
    if (!RTPSDomain::removeRTPSWriter(writer)) {
        printf("error\n");
        exit(1);
    }

    printf("delete rtps writer history\n");
    delete writerHist;

    printf("delete rtps participant\n");
    if (!RTPSDomain::removeRTPSParticipant(part)) {
        printf("error\n");
        exit(1);
    }

    return 0;

} // main
