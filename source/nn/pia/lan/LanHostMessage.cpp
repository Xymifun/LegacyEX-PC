#include "nn/pia/lan/LanHostMessage.hpp"

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
    cMessageType = 4,
    cLanMessageSize = 12,
    cHeaderSize = 16
};
}

LanHostMessage::LanHostMessage() : m_sessionId(0), m_connectionInfo()
{
    m_messageType = cMessageType;
}

LanHostMessage::LanHostMessage(unsigned int sessionId, const transport::StationConnectionInfo& connectionInfo)
    : m_sessionId(sessionId),
      m_connectionInfo(connectionInfo)
{
    m_messageType = cMessageType;
}

Result LanHostMessage::Serialize(unsigned char* buffer, unsigned int* size) const
{
    Result result = LanMessage::Serialize(buffer, size);
    if (result != 0)
        return result;

    unsigned char* p = buffer + *size;
    unsigned int sessionId = m_sessionId;
    p[0] = (sessionId >> 24) & 0xff;
    p[1] = (sessionId >> 16) & 0xff;
    p[2] = (sessionId >> 8) & 0xff;
    p[3] = sessionId & 0xff;
    p += sizeof(sessionId);
    unsigned int connectionInfoSize = 0;
    result = m_connectionInfo.Serialize(p, &connectionInfoSize, m_connectionInfo.GetSerializedSize());
    p += connectionInfoSize;
    if (result != 0)
        return result;

    *size = p - buffer;
    return 0;
}

Result LanHostMessage::Deserialize(const unsigned char* buffer)
{
    Result result = LanMessage::Deserialize(buffer);
    if (result != 0)
        return result;

    const unsigned char* p = buffer + cLanMessageSize;
    m_sessionId = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
    p += sizeof(m_sessionId);
    result = m_connectionInfo.Deserialize(p);
    p += m_connectionInfo.GetSerializedSize();
    if (result != 0)
        return result;

    return 0;
}

unsigned int LanHostMessage::GetSerializedSize() const
{
    return m_connectionInfo.GetSerializedSize() + cHeaderSize;
}

}
}
}
