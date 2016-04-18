#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_config.h"

bool TestConfig::parse_args(int argc, char *argv[], Config &config)
{
    char *endptr;
    int c;
    while ((c = getopt(argc, argv, "h:p:q:r:?")) != -1) {

        switch (c) {

        case 'h': // history (depth)
            config.history = strtol(optarg, &endptr, 0);
            if (optarg == NULL || *optarg == '\0' || *endptr != '\0') {
                return false;
            }
            break;

        case 'p': // print (interval, seconds)
            config.print_s = strtol(optarg, &endptr, 0);
            if (optarg == NULL || *optarg == '\0' || *endptr != '\0') {
                return false;
            }
            break;

        case 'q': // qos
            if (strcmp(optarg, "b") == 0) {
                config.reliable = false;
            } else if (strcmp(optarg, "r") == 0) {
                config.reliable = true;
            } else {
                return false;
            }
            break;

        case 'r': // rate
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
    printf("usage: %s [-q b|r] [-r <rate> ] [-h <history>] [-p <print_s>] [-?]\n", name);
    exit(1);
}
