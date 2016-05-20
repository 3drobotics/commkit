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

static const char *prog = "test_pub_commkit";

static void onConnect(const commkit::PublisherPtr pub)
{
    cout << "subscriber connected (" << pub->matchedSubscribers() << ")" << endl;
}

static void onDisconnect(const commkit::PublisherPtr pub)
{
    cout << "subscriber disconnected (" << pub->matchedSubscribers() << ")" << endl;
}

int main(int argc, char *argv[])
{
    cout << "built " __DATE__ " " __TIME__ << endl;

    TestConfig::Config config;
    if (!TestConfig::parseArgs(argc, argv, config)) {
        TestConfig::usage(prog);
    }

    eprosima::Log::setVerbosity(eprosima::VERB_ERROR);

    Resources resources;

    cout << "create node" << endl;
    commkit::Node node;
    if (!node.init(prog)) {
        cerr << "error" << endl;
        exit(1);
    }

    commkit::Topic topic(TopicData::topicName, TopicData::topicType, sizeof(TopicData));

    cout << "create publisher" << endl;
    auto pub = node.createPublisher(topic);
    if (pub == nullptr) {
        cerr << "error" << endl;
        exit(1);
    }
    pub->onSubscriberConnected.connect(&onConnect);
    pub->onSubscriberDisconnected.connect(&onDisconnect);
    commkit::PublicationOpts pubOpts;
    pubOpts.reliable = config.reliable;
    pubOpts.history = config.history;
    if (!pub->init(pubOpts)) {
        cerr << "error" << endl;
        exit(1);
    }

    commkit::clock::time_point timeZero = commkit::clock::now();

    commkit::clock::time_point pubNext = timeZero;
    commkit::clock::duration pubInterval =
        commkit::clock::duration(std::chrono::nanoseconds(1000000000ull / config.rate));

    commkit::clock::time_point printNext = timeZero;
    commkit::clock::duration printInterval =
        commkit::clock::duration(std::chrono::seconds(config.print_s));

    uint32_t sequence = 0;

    while (true) {

        std::this_thread::sleep_until(pubNext);

        // time at this moment is intended to be pubNext;
        // base calculations on that for consistency

        uint8_t *pubBuf;
        if (pub->reserve(&pubBuf, sizeof(TopicData))) {
            TopicData topicData;
            memset(&topicData, 0, sizeof(TopicData));
            memcpy(pubBuf, &topicData, sizeof(TopicData));
            pub->publishReserved(pubBuf, sizeof(TopicData));
        } else {
            cout << "can't reserve publish buffer (TopicData)" << endl;
        }

        if (pubNext >= printNext) {
            resources.sample();
            double cpu = resources.cpuLoad();
            std::ios::fmtflags f(cout.flags()); // save state
            cout << setw(18) << left << prog << right << " " << setw(10) << fixed << setprecision(3)
                 << commkit::toDouble(pubNext - timeZero) << ": " << setw(6) << sequence << " "
                 << setw(5) << fixed << setprecision(1) << cpu * 100.0 << "%" << endl;
            cout.flags(f); // restore state
            printNext += printInterval;
        }

        sequence++;
        pubNext += pubInterval;
    }

    return 0;

} // main
