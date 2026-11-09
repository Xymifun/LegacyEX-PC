#include "nn/pia/lan/LanSelfKeepAliveMessage.hpp"

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
    cMessageType = 7,
    cSerializedSize = 12
};
}

LanSelfKeepAliveMessage::LanSelfKeepAliveMessage()
{
    m_messageType = cMessageType;
}

Result LanSelfKeepAliveMessage::Serialize(unsigned char* buffer, unsigned int* size) const
{
    Result result = LanMessage::Serialize(buffer, size);
    if (result != 0)
        return result;

    return 0;
}

Result LanSelfKeepAliveMessage::Deserialize(const unsigned char* buffer)
{
    Result result = LanMessage::Deserialize(buffer);
    if (result != 0)
        return result;

    return 0;
}

unsigned int LanSelfKeepAliveMessage::GetSerializedSize() const
{
    return cSerializedSize;
}

}
}
}
