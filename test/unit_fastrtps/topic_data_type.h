
#include <cstring>

#include <fastrtps/TopicDataType.h>

#include "topic_data.h"

using namespace eprosima::fastrtps;

class TestTopicDataType : public TopicDataType
{

public:
    TestTopicDataType()
    {
        setName(TopicData::topicType);
        m_typeSize = sizeof(TopicData);
        m_isGetKeyDefined = false;
    }

    bool serialize(void *data, SerializedPayload_t *payload)
    {
        std::memcpy(payload->data, data, sizeof(TopicData));
        payload->length = sizeof(TopicData);
        return true;
    }

    bool deserialize(SerializedPayload_t *payload, void *data)
    {
        memcpy(data, payload->data, sizeof(TopicData));
        return true;
    }

    void *createData()
    {
        return new TopicData();
    }

    void deleteData(void *data)
    {
        delete (TopicData *)data;
    }
};
