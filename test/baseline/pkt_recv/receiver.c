#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "receiver.h"
#include "util.h"

void *recv_thread(void *void_arg)
{
    struct recv_thread_args *arg = (struct recv_thread_args *)void_arg;
    char *buf;
    int fd;
    struct sockaddr_in loc_addr;
    struct sockaddr_in rem_addr;
    socklen_t rem_addr_len;
    ssize_t num_bytes;
    unsigned msg_num;
    unsigned next_msg_num = 0;
    unsigned gap;

    if (arg->priority != 0) {
        struct sched_param param;
        memset(&param, 0, sizeof(param));
        param.sched_priority = arg->priority;
        if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
            perror("pthread_setschedparam");
            return NULL;
        }
    }

    if ((buf = malloc(arg->length)) == NULL) {
        perror("malloc");
        return NULL;
    }

    fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("socket");
        return NULL;
    }

    memset(&loc_addr, 0, sizeof(loc_addr));
    loc_addr.sin_family = AF_INET;
    loc_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    loc_addr.sin_port = htons(arg->port);

    if (bind(fd, (struct sockaddr *)&loc_addr, sizeof(loc_addr)) != 0) {
        perror("bind");
        close(fd);
        return NULL;
    }

    while (1) {

        memset(&rem_addr, 0, sizeof(rem_addr));
        rem_addr_len = sizeof(rem_addr);
        num_bytes = recvfrom(fd, buf, arg->length, 0, (struct sockaddr *)&rem_addr, &rem_addr_len);
        if (num_bytes < 0) {
            perror("recv");
            close(fd);
            return NULL;
        }

        msg_num = *(unsigned *)buf;
        if (next_msg_num == 0)
            next_msg_num = msg_num;

        __atomic_add_fetch(&arg->received, 1, __ATOMIC_SEQ_CST);

        gap = msg_num - next_msg_num;
        __atomic_add_fetch(&arg->missed, gap, __ATOMIC_SEQ_CST);

        next_msg_num = msg_num + 1;

    } /* for (msg_cnt...) */

    return NULL;

} /* recv_thread */
