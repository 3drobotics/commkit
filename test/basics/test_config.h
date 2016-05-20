#pragma once

namespace TestConfig
{

// default rate, messages/second
constexpr unsigned defaultRate = 500;

// number of messages to send/receive before quitting (-1 = infinite)
constexpr int defaultCount = -1;

// default history depth, messages
constexpr unsigned defaultHistory = 1000;

// default print interval, seconds
constexpr unsigned defaultPrint_s = 1;

// default lease and renew times (-1 means infinite/never)
constexpr double defaultLease_s = -1;
constexpr double defaultRenew_s = -1;

struct Config {
    bool reliable;
    unsigned rate;
    int count;
    unsigned history;
    unsigned print_s;
    double lease_s;
    double renew_s;
    Config()
        : reliable(false), rate(defaultRate), count(defaultCount), history(defaultHistory),
          print_s(defaultPrint_s), lease_s(defaultLease_s), renew_s(defaultRenew_s)
    {
    }
};

bool parseArgs(int argc, char *argv[], Config &config);

void usage(const char *name);
};
