#pragma once

namespace TestConfig {

// default rate, messages/second
constexpr unsigned default_rate = 500;

// default history depth, messages
constexpr unsigned default_history = 1000;

// default print interval, seconds
constexpr unsigned default_print_s = 1;

struct Config {
    bool reliable;
    unsigned rate;
    unsigned history;
    unsigned print_s;
    Config() : reliable(false), rate(default_rate), history(default_history), print_s(default_print_s)
    {
    }
};

bool parse_args(int argc, char *argv[], Config &config);

void usage(const char *name);

};
