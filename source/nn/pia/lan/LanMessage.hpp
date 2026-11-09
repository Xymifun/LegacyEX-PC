#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMESSAGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMESSAGE_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/ISerializable.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanMessage : public common::ISerializable
{
public:
    LanMessage();
    virtual ~LanMessage() {}
    virtual Result Serialize(unsigned char* buffer, unsigned int* size) const;
    virtual Result Deserialize(const unsigned char* buffer);
    virtual unsigned int GetSerializedSize() const;
    virtual void Trace(unsigned long long value);

    unsigned int m_messageType;
    unsigned char m_payload[11];
};

}
}
}

#endif
