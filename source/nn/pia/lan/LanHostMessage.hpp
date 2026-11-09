#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_HOST_MESSAGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_HOST_MESSAGE_HPP

#include "nn/pia/lan/LanMessage.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanHostMessage : public LanMessage
{
public:
    LanHostMessage();
    LanHostMessage(unsigned int sessionId, const transport::StationConnectionInfo& connectionInfo);
    virtual ~LanHostMessage() {}
    virtual Result Serialize(unsigned char* buffer, unsigned int* size) const;
    virtual Result Deserialize(const unsigned char* buffer);
    virtual unsigned int GetSerializedSize() const;

    unsigned int m_sessionId;
    transport::StationConnectionInfo m_connectionInfo;
};

}
}
}

#endif
