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
uint64_t print_interval_ns;
uint64_t print_next_ns;

static Resources resources;

// sequence number in last packet received
static uint32_t last_seq = UINT32_MAX;

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

            TopicData topic_data;
            memcpy(&topic_data, cc->serializedPayload.data, sizeof(topic_data));

            // look for and print gaps in sequence number
            uint32_t sequence = topic_data.sequence;
            if (last_seq != UINT32_MAX && (sequence - last_seq) > 1) {
                printf("gap: %u %50c\n", sequence - last_seq - 1, ' ');
            }
            last_seq = sequence;

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
        r->getHistory()->remove_change(const_cast<CacheChange_t *>(cc));
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

    // zero time is next one-second boundary that is least 0.1 sec away
    zero_ns = clock_gettime_ns(CLOCK_MONOTONIC) + 1100000000; // 1.1 sec from now
    zero_ns = zero_ns - (zero_ns % 1000000000); // round down (shaves off up to 1 second)
    print_interval_ns = config.print_s * 1000000000ULL;
    print_next_ns = zero_ns;

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
    part_attr.setName("test_participant");
    RTPSParticipant *part = RTPSDomain::createParticipant(part_attr);
    if (part == nullptr) {
        printf("error\n");
        exit(1);
    }

    printf("create rtps reader history\n");
    HistoryAttributes hist_attr;
    hist_attr.payloadMaxSize = sizeof(TopicData);
    // XXX config.history
    ReaderHistory *reader_hist = new ReaderHistory(hist_attr);
    if (reader_hist == nullptr) {
        printf("error\n");
        exit(1);
    }

    printf("create rtps reader\n");
    TestReaderListener reader_listener;
    ReaderAttributes reader_attr;
    reader_attr.times.heartbeatResponseDelay.seconds = 0;
    reader_attr.times.heartbeatResponseDelay.fraction = 4294967 * 50; // ~50 millis;
    reader_attr.endpoint.reliabilityKind = config.reliable ? RELIABLE : BEST_EFFORT;
    RTPSReader *reader =
        RTPSDomain::createRTPSReader(part, reader_attr, reader_hist, &reader_listener);
    if (reader == nullptr) {
        printf("error\n");
        exit(1);
    }

    printf("register rtps reader\n");
    TopicAttributes topic_attr;
    topic_attr.topicKind = NO_KEY;
    topic_attr.topicName = TopicData::topic_name;
    topic_attr.topicDataType = TopicData::topic_type;
    ReaderQos reader_qos;
    if (!part->registerReader(reader, topic_attr, reader_qos)) {
        printf("error\n");
        exit(1);
    }

    printf("ready\n");

    while (true) {
        pause();
    }

    return 0;

} // main
