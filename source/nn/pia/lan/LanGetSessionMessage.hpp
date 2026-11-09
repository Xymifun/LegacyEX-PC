#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_GET_SESSION_MESSAGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_GET_SESSION_MESSAGE_HPP

#include "nn/pia/lan/LanMessage.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanGetSessionMessage : public LanMessage
{
public:
    enum
    {
        cSerializedSize = 16
    };

    LanGetSessionMessage();
    explicit LanGetSessionMessage(unsigned int sessionId);
    virtual ~LanGetSessionMessage() {}
    virtual Result Serialize(unsigned char* buffer, unsigned int* size) const;
    virtual Result Deserialize(const unsigned char* buffer);
    virtual unsigned int GetSerializedSize() const;

    unsigned int m_sessionId;
};

}
}
}

#endif
