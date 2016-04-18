#ifndef RECEIVER_H
#define RECEIVER_H

struct recv_thread_args {
    /* in */
    int length;
    int priority;
    uint16_t port;
    /* out */
    unsigned received;
    unsigned missed;
};

extern void *recv_thread(void *void_arg);

#endif /* RECEIVER_H */
