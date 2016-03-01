#pragma once

#include <fastrtps/TopicDataType.h>

/*
 * This is a bit terrible. Some explanation.
 *
 * The current serialization flow in Fast-RTPS is (roughly) as follows:
 *   TopicData -> publish() -> serialize() -> send()
 *
 * This means we'd need to create a separate struct/class for each
 * TopicData type that we want to (de)serialize, which is often cumbersome.
 *
 * Eventually, a better flow would be (see https://github.com/eProsima/Fast-RTPS/issues/21):
 *   serialize() -> publish() -> send()
 *
 * In this case, there's no requirement to define a new type to be passed into
 * publish(), if it is not required by serialize().
 */

struct ByteBufTopicData {
    ByteBufTopicData() : buf(nullptr), len(0), cap(0)
    {
    }

    bool write(const uint8_t *b, size_t sz)
    {
        if (ensureCap(sz)) {
            memcpy(buf, b, sz);
            len = sz;
            return true;
        }
        return false;
    }

    size_t read(uint8_t *b, size_t maxlen)
    {
        size_t n = std::min(len, maxlen);
        memcpy(b, buf, n);
        return n;
    }

    bool ensureCap(size_t sz)
    {
        if (cap < sz) {
            if (buf) {
                free(buf);
            }
            buf = static_cast<uint8_t *>(calloc(1, sz));
            if (buf == nullptr) {
                len = cap = 0;
                return false;
            }
            cap = sz;
        }
        return true;
    }

    uint8_t *buf;
    size_t len;
    size_t cap;
};

class ByteBufTopicDataType : public eprosima::fastrtps::TopicDataType
{
public:
    ByteBufTopicDataType()
    {
        m_typeSize = 0;
        m_isGetKeyDefined = false;
    }

    // setName() is defined on TopicDataType

    void setSize(uint32_t sz)
    {
        m_typeSize = sz;
    }

    bool serialize(void *data, eprosima::fastrtps::rtps::SerializedPayload_t *payload)
    {
        auto bb = static_cast<ByteBufTopicData *>(data);
        payload->length = bb->read(payload->data, payload->max_size);
        return true;
    }

    bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t *payload, void *data)
    {
        auto bb = static_cast<ByteBufTopicData *>(data);
        bb->write(payload->data, payload->length);
        return true;
    }

    void *createData()
    {
        return new ByteBufTopicData();
    }

    void deleteData(void *data)
    {
        delete static_cast<ByteBufTopicData *>(data);
    }
};
