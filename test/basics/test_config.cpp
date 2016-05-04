#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_config.h"

bool TestConfig::parse_args(int argc, char *argv[], Config &config)
{
    char *endptr;
    int c;
    while ((c = getopt(argc, argv, "a:h:l:n:p:q:r:?")) != -1) {

        switch (c) {

        case 'a': // announce period (seconds, double)
            if (optarg == NULL || *optarg == '\0') {
                return false;
            }
            config.renew_s = strtod(optarg, &endptr);
            if (*endptr != '\0') {
                return false;
            }
            break;

        case 'h': // history (depth, integer)
            config.history = strtol(optarg, &endptr, 0);
            if (optarg == NULL || *optarg == '\0' || *endptr != '\0') {
                return false;
            }
            break;

        case 'l': // lease duration (seconds, double)
            if (optarg == NULL || *optarg == '\0') {
                return false;
            }
            config.lease_s = strtod(optarg, &endptr);
            if (*endptr != '\0') {
                return false;
            }
            break;

        case 'n': // count (messages, integer)
            config.count = strtol(optarg, &endptr, 0);
            if (optarg == NULL || *optarg == '\0' || *endptr != '\0') {
                return false;
            }
            break;

        case 'p': // print (interval, seconds, integer)
            config.print_s = strtol(optarg, &endptr, 0);
            if (optarg == NULL || *optarg == '\0' || *endptr != '\0') {
                return false;
            }
            break;

        case 'q': // qos ('b' or 'r')
            if (strcmp(optarg, "b") == 0) {
                config.reliable = false;
            } else if (strcmp(optarg, "r") == 0) {
                config.reliable = true;
            } else {
                return false;
            }
            break;

        case 'r': // rate (messages/second, integer)
            config.rate = strtol(optarg, &endptr, 0);
            if (optarg == NULL || *optarg == '\0' || *endptr != '\0') {
                return false;
            }
            break;

        case '?': // help
            // caller expected to print usage
            return false;

        default:
            return false;

        } // switch

    } // while

    return true;
}

void TestConfig::usage(const char *name)
{
    printf("usage: %s [options]\n", name);
    printf("       [-q b|r]         qos, best effort or reliable\n");
    printf("       [-r count]       message rate, messages/sec\n");
    printf("       [-n count]       messages to pub or sub\n");
    printf("       [-h count]       history depth\n");
    printf("       [-l duration]    lease duration, seconds\n");
    printf("       [-a interval]    announce interval, seconds\n");
    printf("       [-p interval]    print interval, seconds\n");
    printf("       [-?]             print this\n");
    exit(1);
}
