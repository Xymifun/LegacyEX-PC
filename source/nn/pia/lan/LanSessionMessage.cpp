#include "nn/pia/lan/LanSessionMessage.hpp"

#include <coreinit/memory.h>

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
    cMessageType = 6,
    cLanMessageSize = 12,
    cHeaderSize = 24
};
}

LanSessionMessage::LanSessionMessage() : m_sessionId(0), m_sequence(0), m_index(0), m_count(0), m_fragmentSize(0)
{
    m_messageType = cMessageType;
}

LanSessionMessage::LanSessionMessage(unsigned int sessionId, unsigned short sequence, unsigned char index,
                                     unsigned char count, const unsigned char* fragment, unsigned int fragmentSize)
    : m_sessionId(sessionId),
      m_sequence(sequence),
      m_index(index),
      m_count(count),
      m_fragmentSize(fragmentSize)
{
    m_messageType = cMessageType;
    OSBlockMove(m_fragment, fragment, fragmentSize, FALSE);
}

Result LanSessionMessage::Serialize(unsigned char* buffer, unsigned int* size) const
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
    unsigned short sequence = m_sequence;
    p[0] = (sequence >> 8) & 0xff;
    p[1] = sequence & 0xff;
    p += sizeof(sequence);
    *p = m_index;
    p += sizeof(m_index);
    *p = m_count;
    p += sizeof(m_count);
    unsigned int fragmentSize = m_fragmentSize;
    p[0] = (fragmentSize >> 24) & 0xff;
    p[1] = (fragmentSize >> 16) & 0xff;
    p[2] = (fragmentSize >> 8) & 0xff;
    p[3] = fragmentSize & 0xff;
    p += sizeof(fragmentSize);
    OSBlockMove(p, m_fragment, m_fragmentSize, FALSE);
    p += m_fragmentSize;
    *size = p - buffer;
    return 0;
}

Result LanSessionMessage::Deserialize(const unsigned char* buffer)
{
    Result result = LanMessage::Deserialize(buffer);
    if (result != 0)
        return result;

    const unsigned char* p = buffer + cLanMessageSize;
    m_sessionId = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
    p += sizeof(m_sessionId);
    m_sequence = (p[0] << 8) | p[1];
    p += sizeof(m_sequence);
    m_index = *p;
    p += sizeof(m_index);
    m_count = *p;
    p += sizeof(m_count);
    m_fragmentSize = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
    p += sizeof(m_fragmentSize);
    OSBlockMove(m_fragment, p, m_fragmentSize, FALSE);
    return 0;
}

unsigned int LanSessionMessage::GetSerializedSize() const
{
    return m_fragmentSize + cHeaderSize;
}

}
}
}
