#include "nn/pia/lan/LanGetSessionMessage.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

namespace
{
enum
{
    cMessageType = 5
};
}

LanGetSessionMessage::LanGetSessionMessage() : m_sessionId(0)
{
    m_messageType = cMessageType;
}

LanGetSessionMessage::LanGetSessionMessage(unsigned int sessionId) : m_sessionId(sessionId)
{
    m_messageType = cMessageType;
}

Result LanGetSessionMessage::Serialize(unsigned char* buffer, unsigned int* size) const
{
    Result result = LanMessage::Serialize(buffer, size);
    if (result != 0)
        return result;

    unsigned int sessionId = m_sessionId;
    unsigned char* out = buffer + *size;
    out[0] = (sessionId >> 24) & 0xff;
    out[1] = (sessionId >> 16) & 0xff;
    out[2] = (sessionId >> 8) & 0xff;
    out[3] = sessionId & 0xff;
    out += 4;
    *size = out - buffer;
    return 0;
}

Result LanGetSessionMessage::Deserialize(const unsigned char* buffer)
{
    Result result = LanMessage::Deserialize(buffer);
    if (result != 0)
        return result;

    m_sessionId = (static_cast<unsigned int>(buffer[12]) << 24) | (static_cast<unsigned int>(buffer[13]) << 16)
                  | (static_cast<unsigned int>(buffer[14]) << 8) | static_cast<unsigned int>(buffer[15]);
    return 0;
}

unsigned int LanGetSessionMessage::GetSerializedSize() const
{
    return cSerializedSize;
}

}
}
}
