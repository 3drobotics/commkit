#include <commkit/commkit.h>

struct commkit_node {
    commkit::Node node;
};

struct commkit_subscriber {
    commkit::SubscriberPtr s;
};

struct commkit_publisher {
    commkit::PublisherPtr p;
};

commkit_node* commkit_node_create(const char* name) {
  commkit_node* n = new commkit_node;
  n->node.init(name);

  return n;
}

void commkit_node_destroy(commkit_node* node) {
  delete node;
}

commkit_subscriber* commkit_subscriber_create(commkit_node* node, const char* topic_name, const char* datatype, const commkit_subscriber_opts *opts) {
  commkit::Topic t(topic_name, datatype, 4096);
  commkit_subscriber* s = new commkit_subscriber;
  s->s = node->node.createSubscriber(t);
  
  commkit::SubscriptionOpts sub_opts;
  sub_opts.reliable = opts->reliable;
  
  if (!s->s->init(sub_opts)) {
      return nullptr;
  }
  
  return s;
}

void commkit_subscriber_destroy(commkit_subscriber* sub) {
  delete sub;
}

commkit_publisher* commkit_publisher_create(commkit_node* node, const char* topic_name, const char* datatype, const commkit_publisher_opts *opts) {
  commkit::Topic t(topic_name, datatype, 4096);
  commkit_publisher* p = new commkit_publisher;
  p->p = node->node.createPublisher(t);
  
  commkit::PublicationOpts pub_opts;
  pub_opts.reliable = opts->reliable;
  pub_opts.history = 0;
  
  if (!p->p->init(pub_opts)) {
      return nullptr;
  }
  
  return p;
}

void commkit_publisher_destroy(commkit_publisher* pub) {
  delete pub;
}

void commkit_wait_for_message(commkit_subscriber* sub) {
  sub->s->waitForMessage();
}

int commkit_matched_publishers(commkit_subscriber* sub) {
  return sub->s->matchedPublishers();
}

bool commkit_peek(commkit_subscriber* sub, commkit_payload* payload) {
  commkit::Payload p;
  if (sub->s->peek(&p)) {
    payload->bytes = p.bytes;
    payload->len = p.len;
    payload->sequence = p.sequence;
    payload->source_timestamp = std::chrono::time_point_cast<std::chrono::nanoseconds>(p.sourceTimestamp).time_since_epoch().count();
    return true;
  } else {
    return false;
  }
}

bool commkit_take(commkit_subscriber* sub, commkit_payload* payload) {
  commkit::Payload p;
  if (sub->s->take(&p)) {
    payload->bytes = p.bytes;
    payload->len = p.len;
    payload->sequence = p.sequence;
    payload->source_timestamp = std::chrono::time_point_cast<std::chrono::nanoseconds>(p.sourceTimestamp).time_since_epoch().count();
    return true;
  } else {
    return false;
  }
}

bool commkit_publish(commkit_publisher* pub, uint8_t* data, size_t len) {
  return pub->p->publish(data, len);
}

int commkit_matched_subscribers(commkit_publisher* pub) {
    return pub->p->matchedSubscribers();
}



