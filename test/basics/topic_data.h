#pragma once

struct TopicData {
    static const char * topic_name;
    static const char * topic_type;
    uint64_t timestamp_ns;
    uint32_t sequence;
    uint32_t data[5];
};

const char * TopicData::topic_name = "TestTopic";
const char * TopicData::topic_type = "TopicData";
