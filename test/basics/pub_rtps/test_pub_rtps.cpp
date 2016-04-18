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
    if (!TestConfig::parse_args(argc, argv, config)) {
        TestConfig::usage(prog);
    }

    eprosima::Log::setVerbosity(eprosima::VERB_ERROR);

    Resources resources;

    printf("create rtps participant\n");
    RTPSParticipantAttributes part_attr;
    part_attr.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
    part_attr.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
    part_attr.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    part_attr.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    part_attr.builtin.domainId = 80;
    part_attr.builtin.leaseDuration = c_TimeInfinite;
    part_attr.sendSocketBufferSize = 8712;
    part_attr.listenSocketBufferSize = 17424;
    part_attr.setName(prog);
    RTPSParticipant *part = RTPSDomain::createParticipant(part_attr);
    if (part == nullptr) {
        printf("error\n");
        exit(1);
    }

    printf("create rtps writer history\n");
    HistoryAttributes hist_attr;
    hist_attr.payloadMaxSize = sizeof(TopicData);
    // XXX config.history
    WriterHistory *writer_hist = new WriterHistory(hist_attr);
    if (writer_hist == nullptr) {
        printf("error\n");
        exit(1);
    }

    printf("create rtps writer\n");
    TestWriterListener writer_listener;
    WriterAttributes writer_attr;
    writer_attr.endpoint.reliabilityKind = config.reliable ? RELIABLE : BEST_EFFORT;
    RTPSWriter *writer =
        RTPSDomain::createRTPSWriter(part, writer_attr, writer_hist, &writer_listener);
    if (writer == nullptr) {
        printf("error\n");
        exit(1);
    }

    printf("register rtps writer\n");
    TopicAttributes topic_attr;
    topic_attr.topicKind = NO_KEY;
    topic_attr.topicName = TopicData::topic_name;
    topic_attr.topicDataType = TopicData::topic_type;
    WriterQos writer_qos;
    if (!part->registerWriter(writer, topic_attr, writer_qos)) {
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

        CacheChange_t *cache_change = writer->new_change(ALIVE);

        TopicData topic_data;
        memset(&topic_data, 0, sizeof(TopicData));
        topic_data.timestamp_ns = pub_next_ns;
        topic_data.sequence = sequence;
        memcpy(cache_change->serializedPayload.data, &topic_data, sizeof(TopicData));
        cache_change->serializedPayload.length = sizeof(TopicData);

        writer_hist->add_change(cache_change);

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

    // XXX unregister writer?

    printf("remove rtps writer\n");
    // XXX does this delete it?
    if (!RTPSDomain::removeRTPSWriter(writer)) {
        printf("error\n");
        exit(1);
    }

    printf("delete rtps writer history\n");
    delete writer_hist;

    printf("delete rtps participant\n");
    if (!RTPSDomain::removeRTPSParticipant(part)) {
        printf("error\n");
        exit(1);
    }

    return 0;

} // main
