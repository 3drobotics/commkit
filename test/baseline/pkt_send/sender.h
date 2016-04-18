#ifndef SENDER_H
#define SENDER_H

#include <arpa/inet.h>

struct send_thread_args {
    /* in */
    struct in_addr dst_ip;
    uint16_t dst_port;
    int length; /* bytes/packet */
    int priority;
    int tos;
    int sock;    /* boolean */
    int bursts;  /* bursts/second */
    int packets; /* packets/burst */
    /* out */
    unsigned sent;
};

extern void *send_thread(void *void_args);

#endif /* SENDER_H */
