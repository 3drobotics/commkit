#pragma once

struct TopicData {
    static const char *topic_name;
    static const char *topic_type;
    TopicData()
    {
        for (int i = 0; i < 4; i++)
            data[i] = i;
    }
    ~TopicData()
    {
        for (int i = 0; i < 4; i++)
            data[i] = -1;
    }
    int32_t data[4];
};

const char *TopicData::topic_name = "TestTopic";
const char *TopicData::topic_type = "TopicData";
