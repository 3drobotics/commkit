#pragma once

struct TopicData {
    static const char *topicName;
    static const char *topicType;
    uint64_t timestamp_ns;
    uint32_t sequence;
    uint32_t data[5];
};

const char *TopicData::topicName = "TestTopic";
const char *TopicData::topicType = "TopicData";
