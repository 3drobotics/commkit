#pragma once

/*
 * Helper to include common apis.
 */
#include <commkit/visibility.h>
#ifdef __cplusplus
#include <commkit/callback.h>
#include <commkit/chrono.h>
#include <commkit/types.h>
#include <commkit/topic.h>
#include <commkit/make_unique_cpp11.h>
#include <commkit/node.h>
#include <commkit/publisher.h>
#include <commkit/subscriber.h>
#include <commkit/rtps.h>

extern "C" {
#endif

typedef struct {
  uint8_t *bytes;
  size_t len;
  int64_t sequence;
  uint64_t source_timestamp;
} commkit_payload;

struct commkit_opts {
  bool reliable;
};

typedef struct commkit_opts commkit_subscriber_opts;
typedef struct commkit_opts commkit_publisher_opts;

typedef struct commkit_node commkit_node;
typedef struct commkit_subscriber commkit_subscriber;
typedef struct commkit_publisher commkit_publisher;

COMMKIT_API commkit_node* commkit_node_create(const char* name);
COMMKIT_API void commkit_node_destroy(commkit_node* node);

COMMKIT_API commkit_subscriber* commkit_subscriber_create(commkit_node* node, const char* topic_name, const char* datatype, const commkit_subscriber_opts *opts);
COMMKIT_API void commkit_subscriber_destroy(commkit_subscriber* sub);

COMMKIT_API commkit_publisher* commkit_publisher_create(commkit_node* node, const char* topic_name, const char* datatype, const commkit_publisher_opts *opts);
COMMKIT_API void commkit_publisher_destroy(commkit_publisher* pub);

COMMKIT_API void commkit_wait_for_message(commkit_subscriber* sub);
COMMKIT_API int commkit_matched_publishers(commkit_subscriber* sub);
COMMKIT_API bool commkit_peek(commkit_subscriber* sub, commkit_payload* payload);
COMMKIT_API bool commkit_take(commkit_subscriber* sub, commkit_payload* payload);

COMMKIT_API bool commkit_publish(commkit_publisher* pub, uint8_t* data, size_t len);
COMMKIT_API int commkit_matched_subscribers(commkit_publisher* pub);

#ifdef __cplusplus
}
#endif

