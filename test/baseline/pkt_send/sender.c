#include <errno.h>
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

/* forwards */
static void set_ip_tos(int fd, uint8_t tos);
static void set_sock_prio(int fd);

void *send_thread(void *void_arg)
{
    struct send_thread_args *arg = (struct send_thread_args *)void_arg;
    int fd;
    struct sockaddr_in loc_addr;
    struct sockaddr_in rem_addr;
    char *buf;
    uint64_t now_ns;
    uint64_t msg_time_ns;
    uint64_t interval_ns;
    int i;
    unsigned pkt_num = 0;

    if ((buf = malloc(arg->length)) == NULL) {
        perror("malloc");
        return NULL;
    }
    for (i = 0; i < arg->length; i++)
        buf[i] = (char)i;

    if (arg->priority != 0) {
        struct sched_param param;
        memset(&param, 0, sizeof(param));
        param.sched_priority = arg->priority;
        if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
            perror("pthread_setschedparam");
            return NULL;
        }
    }

    fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("socket");
        return NULL;
    }

    memset(&loc_addr, 0, sizeof(loc_addr));
    loc_addr.sin_family = AF_INET;
    loc_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    loc_addr.sin_port = htons(0);

    if (bind(fd, (struct sockaddr *)&loc_addr, sizeof(loc_addr)) != 0) {
        perror("bind");
        close(fd);
        return NULL;
    }

    if (arg->tos != -1)
        set_ip_tos(fd, arg->tos);

    if (arg->sock)
        set_sock_prio(fd);

    interval_ns = 1000000000ULL / arg->bursts;

    memset(&rem_addr, 0, sizeof(rem_addr));
    rem_addr.sin_family = AF_INET;
    rem_addr.sin_addr.s_addr = arg->dst_ip.s_addr;
    rem_addr.sin_port = htons(arg->dst_port);

    msg_time_ns = clock_gettime_ns(CLOCK_MONOTONIC);

    while (1) {

        msg_time_ns = msg_time_ns + interval_ns;

        /* sleep until the next message time */

        now_ns = clock_gettime_ns(CLOCK_MONOTONIC);
        if (msg_time_ns > now_ns) {
            unsigned delay_us = (msg_time_ns - now_ns) / 1000;
            usleep(delay_us);
        }

        /* send burst */

        for (i = 0; i < arg->packets; i++) {
            /*printf("send %u\n", pkt_num);*/
            *(unsigned *)buf = pkt_num++;
            if (sendto(fd, buf, arg->length, 0, (struct sockaddr *)&rem_addr, sizeof(rem_addr)) !=
                arg->length) {
                perror("sendto");
                close(fd);
                return NULL;
            }
            __atomic_add_fetch(&arg->sent, 1, __ATOMIC_SEQ_CST);
        }

    } /* while (1) */

} /* send_thread */

/*
 * Set IPPROTO_IP/IP_TOS
 * This sets the IP header TOS field.
 * netinet/ip.h says this is deprecated; do it with DSCP/CS instead.
 *
 * With "bulk" traffic saturating the outbound link, and "priority" traffic
 * using this setting:
 * tos=0x10 is not sufficient to push priority traffic through
 * tos=0xd0 is sufficient
 */
static void set_ip_tos(int fd, uint8_t tos)
{
    uint8_t tos0, tos1;
    socklen_t tos_len;

    tos_len = sizeof(tos0);
    if (getsockopt(fd, IPPROTO_IP, IP_TOS, &tos0, &tos_len) != 0) {
        perror("getsockopt");
        close(fd);
        exit(1);
    }
    if (tos_len != 1) {
        printf("tos_len != 1\n");
        close(fd);
        exit(1);
    }

    if (setsockopt(fd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos)) != 0) {
        perror("setsockopt");
        close(fd);
        exit(1);
    }

    tos_len = sizeof(tos1);
    if (getsockopt(fd, IPPROTO_IP, IP_TOS, &tos1, &tos_len) != 0) {
        perror("getsockopt");
        close(fd);
        exit(1);
    }

    printf("tos0 = 0x%02x; set to 0x%02x; tos1 = 0x%02x\n", tos0, tos, tos1);

} /* set_ip_tos */

/*
 * Set SOL_SOCKET/SO_PRIORITY
 * This does not appear to set the IP header's TOS field.
 */
static void set_sock_prio(int fd)
{
#if (!defined __MACH__ || !defined __APPLE__)
    int so_prio;
    socklen_t so_prio_len;

    so_prio = -99;
    so_prio_len = sizeof(so_prio);
    if (getsockopt(fd, SOL_SOCKET, SO_PRIORITY, &so_prio, &so_prio_len) != 0) {
        perror("getsockopt");
        close(fd);
        exit(1);
    }
    printf("so_prio is %d; so_prio_len is %d\n", so_prio, so_prio_len);

    so_prio = 6;
    if (setsockopt(fd, SOL_SOCKET, SO_PRIORITY, &so_prio, sizeof(so_prio_len)) != 0) {
        perror("setsockopt");
        close(fd);
        exit(1);
    }
    printf("so_prio set to %d\n", 6);

    so_prio = -99;
    so_prio_len = sizeof(so_prio);
    if (getsockopt(fd, SOL_SOCKET, SO_PRIORITY, &so_prio, &so_prio_len) != 0) {
        perror("getsockopt");
        close(fd);
        exit(1);
    }
    printf("so_prio is %d; so_prio_len is %d\n", so_prio, so_prio_len);
#endif
} /* set_sock_prio */
