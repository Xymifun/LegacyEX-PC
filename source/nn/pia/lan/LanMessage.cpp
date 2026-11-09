#include "nn/pia/lan/LanMessage.hpp"

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
    cResultInvalidArgument = 0x10c07,
    cSerializedSize = 12
};
}

LanMessage::LanMessage() : m_messageType(0)
{
    OSBlockSet(m_payload, 0, sizeof(m_payload));
}

Result LanMessage::Serialize(unsigned char* buffer, unsigned int* size) const
{
    if (buffer == 0)
        return cResultInvalidArgument;
    if (size == 0)
        return cResultInvalidArgument;

    unsigned char* p = buffer;
    *p = static_cast<unsigned char>(m_messageType);
    p += 1;
    OSBlockMove(p, m_payload, sizeof(m_payload), FALSE);
    p += sizeof(m_payload);
    *size = p - buffer;
    return 0;
}

Result LanMessage::Deserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return cResultInvalidArgument;

    const unsigned char* p = buffer;
    m_messageType = *p;
    p += 1;
    OSBlockMove(m_payload, p, sizeof(m_payload), FALSE);
    return 0;
}

unsigned int LanMessage::GetSerializedSize() const
{
    return cSerializedSize;
}

void LanMessage::Trace(unsigned long long)
{
}

}
}
}
