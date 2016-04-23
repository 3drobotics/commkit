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
    if (!TestConfig::parse_args(argc, argv, config)) {
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

    commkit::Topic topic(TopicData::topic_name, TopicData::topic_type, sizeof(TopicData));

    cout << "create publisher" << endl;
    auto pub = node.createPublisher(topic);
    if (pub == nullptr) {
        cerr << "error" << endl;
        exit(1);
    }
    pub->onSubscriberConnected.connect(&onConnect);
    pub->onSubscriberDisconnected.connect(&onDisconnect);
    commkit::PublicationOpts pub_opts;
    pub_opts.reliable = config.reliable;
    pub_opts.history = config.history;
    if (!pub->init(pub_opts)) {
        cerr << "error" << endl;
        exit(1);
    }

    commkit::clock::time_point time_zero = commkit::clock::now();

    commkit::clock::time_point pub_next = time_zero;
    commkit::clock::duration pub_interval =
        commkit::clock::duration(std::chrono::nanoseconds(1000000000ull / config.rate));

    commkit::clock::time_point print_next = time_zero;
    commkit::clock::duration print_interval =
        commkit::clock::duration(std::chrono::seconds(config.print_s));

    uint32_t sequence = 0;

    while (true) {

        std::this_thread::sleep_until(pub_next);

        // time at this moment is intended to be pub_next;
        // base calculations on that for consistency

        uint8_t *pub_buf;
        if (pub->reserve(&pub_buf, sizeof(TopicData))) {
            TopicData topic_data;
            memset(&topic_data, 0, sizeof(TopicData));
            memcpy(pub_buf, &topic_data, sizeof(TopicData));
            pub->publishReserved(pub_buf, sizeof(TopicData));
        } else {
            cout << "can't reserve publish buffer (TopicData)" << endl;
        }

        if (pub_next >= print_next) {
            resources.sample();
            double cpu = resources.cpu_load();
            std::ios::fmtflags f(cout.flags()); // save state
            cout << setw(18) << left << prog << right << " " << setw(10) << fixed << setprecision(3)
                 << commkit::toDouble(pub_next - time_zero) << ": " << setw(6) << sequence << " "
                 << setw(5) << fixed << setprecision(1) << cpu * 100.0 << "%" << endl;
            cout.flags(f); // restore state
            print_next += print_interval;
        }

        sequence++;
        pub_next += pub_interval;
    }

    return 0;

} // main
