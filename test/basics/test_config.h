#pragma once

namespace TestConfig
{

// default rate, messages/second
constexpr unsigned default_rate = 500;

// number of messages to send/receive before quitting (-1 = infinite)
constexpr int default_count = -1;

// default history depth, messages
constexpr unsigned default_history = 1000;

// default print interval, seconds
constexpr unsigned default_print_s = 1;

// default lease and renew times (-1 means infinite/never)
constexpr double default_lease_s = -1;
constexpr double default_renew_s = -1;

struct Config {
    bool reliable;
    unsigned rate;
    int count;
    unsigned history;
    unsigned print_s;
    double lease_s;
    double renew_s;
    Config()
        : reliable(false), rate(default_rate), count(default_count), history(default_history),
          print_s(default_print_s), lease_s(default_lease_s), renew_s(default_renew_s)
    {
    }
};

bool parse_args(int argc, char *argv[], Config &config);

void usage(const char *name);
};
