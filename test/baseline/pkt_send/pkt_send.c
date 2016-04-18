#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sched.h>
#include "sender.h"
#include "time_util.h"

static void usage(void)
{
    printf("pkt_send [options]\n");
    printf("    -a <name>               name to print in output\n");
    printf("    -d <dest-ip>            IP to send packets to\n");
    printf("    -p <dest-port>          UDP port to send packets to\n");
    printf("    -b <bursts-per-second>  bursts per second to send (1)\n");
    printf("    -n <packets-per-burst>  packets per burst (1)\n");
    printf("    -l <bytes-per-packet>   bytes per packet (100)\n");
    printf("    -s                      set socket priority (see code)\n");
    printf("    -r <real-time-priority> set thread real time priority\n");
    printf("    -t <tos>                set IP TOS bits\n");
    printf("    -i <interval>           report interval\n");
    printf("    -h                      help\n");
    exit(1);
}

/* options */
static char const *opt_name = NULL; /* -a <name> */
static char const *opt_ip = NULL;   /* -d <dest-ip> */
static short opt_port = 0;          /* -p <dest-port> UDP port to send to */
static int opt_bursts = 1;          /* -b <bursts-per-second> */
static int opt_packets = 1;         /* -n <packets-per-burst> */
static int opt_length = 100;        /* -l <bytes-per-packet> */
static int opt_sock = 0;            /* -s */
static int opt_prio = 0;            /* -r <prio> */
static int opt_tos = -1;            /* -t <tos> */
static int opt_interval_s = 1;      /* -i <report-interval> */
static int opt_help = 0;            /* -h */

static pthread_t send_tid;

static void parse_options(int argc, char *const argv[])
{
    int c;
    char const *opts = "a:d:p:b:n:l:sr:t:i:h";

    while ((c = getopt(argc, argv, opts)) != -1) {
        switch (c) {
        case 'a':
            opt_name = optarg;
            break;
        case 'd':
            opt_ip = optarg;
            break;
        case 'p':
            opt_port = atoi(optarg);
            break;
        case 'b':
            opt_bursts = atoi(optarg);
            break;
        case 'n':
            opt_packets = atoi(optarg);
            break;
        case 'l':
            opt_length = atoi(optarg);
            break;
        case 's':
            opt_sock = 1;
            break;
        case 'r':
            opt_prio = atoi(optarg);
            break;
        case 't':
            opt_tos = strtol(optarg, NULL, 0);
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
    uint64_t report_time_ns = 0;
    uint64_t report_interval_ns;
    unsigned sent_last = 0;
    unsigned sent_new;
    struct send_thread_args arg;

    parse_options(argc, argv);

    if (opt_help)
        usage();

    if (opt_port == 0)
        usage();

    if (opt_ip == NULL)
        usage();

    memset(&arg, 0, sizeof(arg));

    if (inet_pton(AF_INET, opt_ip, &arg.dst_ip) != 1)
        usage();

    arg.dst_port = opt_port;
    arg.length = opt_length;
    arg.priority = opt_prio;
    arg.tos = opt_tos;
    arg.sock = opt_sock;
    arg.bursts = opt_bursts;
    arg.packets = opt_packets;

    if (pthread_create(&send_tid, NULL, send_thread, &arg) != 0) {
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

        sent_new = __atomic_load_n(&arg.sent, __ATOMIC_SEQ_CST);

        printf("%3s:     sent +%4u (%7u)\n", opt_name, sent_new - sent_last, sent_new);

        sent_last = sent_new;
    }

    if (pthread_join(send_tid, NULL) != 0) {
        perror("pthread_join");
        exit(1);
    }

    exit(0);

} /* main */
