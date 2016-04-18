#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sched.h>
#include "receiver.h"
#include "time_util.h"

static void usage(void)
{
    printf("pkt_recv [options]\n");
    printf("    -a <name>               name to print in output\n");
    printf("    -p <src-port>           UDP port to receive on\n");
    printf("    -l <bytes-per-packet>   bytes per packet (10000)\n");
    printf("    -r <real-time-priority> set thread real time priority\n");
    printf("    -i <interval>           report interval\n");
    printf("    -h                      help\n");
    exit(1);
}

/* options */
static char const *opt_name = NULL; /* -a <name> */
static short opt_port = 0;          /* -p <port> */
static int opt_length = 10000;      /* -l <bytes-per-packet> */
static int opt_prio = 0;            /* -r <prio> */
static int opt_interval_s = 1;      /* -i <report-interval> */
static int opt_help = 0;            /* -h */

static pthread_t recv_tid;

static void parse_options(int argc, char *const argv[])
{
    int c;
    char const *opts = "a:p:l:r:i:h";

    while ((c = getopt(argc, argv, opts)) != -1) {
        switch (c) {
        case 'a':
            opt_name = optarg;
            break;
        case 'p':
            opt_port = atoi(optarg);
            break;
        case 'l':
            opt_length = atoi(optarg);
            break;
        case 'r':
            opt_prio = atoi(optarg);
            break;
        case 'i':
            opt_interval_s = atoi(optarg);
            break;
        case 'h':
            opt_help = 1;
            break;
        }
    }

} /* parse_options */

int main(int argc, char *argv[])
{
    struct recv_thread_args arg;
    uint64_t report_time_ns = 0;
    uint64_t report_interval_ns;
    unsigned received_last = 0;
    unsigned received_new;
    unsigned missed_last = 0;
    unsigned missed_new;

    parse_options(argc, argv);

    if (opt_help)
        usage();

    if (opt_port == 0)
        usage();

    arg.length = opt_length;
    arg.priority = opt_prio;
    arg.port = opt_port;

    if (pthread_create(&recv_tid, NULL, recv_thread, &arg) != 0) {
        perror("pthread_create");
        exit(1);
    }

    report_interval_ns = opt_interval_s * 1000000000ULL;
    report_time_ns = clock_gettime_ns(CLOCK_MONOTONIC);

    while (1) {
        report_time_ns += report_interval_ns;

        uint64_t now_ns = clock_gettime_ns(CLOCK_MONOTONIC);
        if (report_time_ns > now_ns)
            usleep((report_time_ns - now_ns) / 1000);

        received_new = __atomic_load_n(&arg.received, __ATOMIC_SEQ_CST);
        missed_new = __atomic_load_n(&arg.missed, __ATOMIC_SEQ_CST);

        printf("%3s: received +%4u (%7u), missed +%4u (%7u)\n", opt_name,
               received_new - received_last, received_new, missed_new - missed_last, missed_new);

        received_last = received_new;
        missed_last = missed_new;
    }

    if (pthread_join(recv_tid, NULL) != 0) {
        perror("pthread_join");
        exit(1);
    }

    exit(0);

} /* main */
