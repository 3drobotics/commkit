#include <unistd.h>

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <thread>

#include <fastrtps/log/Log.h>
#include <commkit/commkit.h>

#include "topic_data.h"
#include "test_config.h"
#include "resources.h"

using std::cerr;
using std::cout;
using std::endl;
using std::fixed;
using std::left;
using std::right;
using std::setprecision;
using std::setw;

static const char *prog = "test_sub_commkit";

commkit::clock::time_point timeZero;
commkit::clock::time_point printNext;
commkit::clock::duration printInterval;

static Resources resources;

// sequence number in last packet received
static int64_t lastSeq = commkit::SEQUENCE_NUMBER_INVALID;

static void onConnect(const commkit::SubscriberPtr sub)
{
    cout << "publisher connected (" << sub->matchedPublishers() << ")" << endl;
}

static void onDisconnect(const commkit::SubscriberPtr sub)
{
    cout << "publisher disconnected (" << sub->matchedPublishers() << ")" << endl;
}

static void onMessage(commkit::SubscriberPtr sub)
{
    commkit::Payload payload;
    while (sub->take(&payload)) {

        commkit::clock::time_point now = commkit::clock::now();

        if (payload.len != sizeof(TopicData)) {
            cout << "message wrong length (expected " << sizeof(TopicData) << ", got "
                 << payload.len << ")" << endl;
            continue;
        }

        TopicData topicData;
        memcpy(&topicData, payload.bytes, sizeof(TopicData));

        // look for and print gaps in sequence number
        if (lastSeq != commkit::SEQUENCE_NUMBER_INVALID && (payload.sequence - lastSeq) != 1) {
            cout << "gap: " << payload.sequence - lastSeq - 1 << endl;
        }
        lastSeq = payload.sequence;

        // look for long latencies (assumes same system pub/sub)
        commkit::clock::duration latency = now - payload.sourceTimestamp;
        if (latency > std::chrono::milliseconds(10)) {
            cout << "latency: " << fixed << setprecision(3) << commkit::toDouble(latency) << endl;
        }

        if (now >= printNext) {
            resources.sample();
            double cpu = resources.cpuLoad();
            std::ios::fmtflags f(cout.flags()); // save state
            cout << setw(18) << left << prog << right << " " << setw(10) << fixed << setprecision(3)
                 << commkit::toDouble(now - timeZero) << ": " << setw(6) << payload.sequence << " "
                 << setw(5) << fixed << setprecision(1) << cpu * 100.0 << "%" << endl;
            cout.flags(f); // restore state
            printNext += printInterval;
        }
    }
}

int main(int argc, char *argv[])
{
    cout << "built " __DATE__ " " __TIME__ << endl;

    TestConfig::Config config;
    if (!TestConfig::parseArgs(argc, argv, config)) {
        TestConfig::usage(prog);
    }

    eprosima::Log::setVerbosity(eprosima::VERB_ERROR);

    printInterval = commkit::clock::duration(std::chrono::seconds(config.print_s));
    timeZero = commkit::clock::now();
    printNext = timeZero;

    cout << "create node" << endl;
    commkit::Node node;
    if (!node.init(prog)) {
        cerr << "error" << endl;
        exit(1);
    }

    commkit::Topic topic(TopicData::topicName, TopicData::topicType, sizeof(TopicData));

    cout << "create subscriber" << endl;
    auto sub = node.createSubscriber(topic);
    if (sub == nullptr) {
        cerr << "error" << endl;
        exit(1);
    }
    sub->onPublisherConnected.connect(&onConnect);
    sub->onPublisherDisconnected.connect(&onDisconnect);
    sub->onMessage.connect(&onMessage);
    commkit::SubscriptionOpts subOpts;
    subOpts.reliable = config.reliable;
    subOpts.history = config.history;
    if (!sub->init(subOpts)) {
        cerr << "error" << endl;
        exit(1);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    cout << "ready" << endl;

    while (true) {
        pause();
    }

    return 0;

} // main
