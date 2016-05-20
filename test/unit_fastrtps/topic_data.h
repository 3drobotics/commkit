#pragma once

struct TopicData {
    static const char *topicName;
    static const char *topicType;
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

const char *TopicData::topicName = "TestTopic";
const char *TopicData::topicType = "TopicData";
