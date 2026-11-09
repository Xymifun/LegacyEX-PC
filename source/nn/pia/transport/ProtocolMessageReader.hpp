#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PROTOCOL_MESSAGE_READER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PROTOCOL_MESSAGE_READER_HPP

#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/transport/ProtocolMessageAccessor.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class Packet;
}
}
}

namespace nn
{
namespace pia
{
namespace transport
{

class ProtocolMessageReader
{
public:
    enum
    {
        cMessageHeaderSize = 24,
        cTerminatorMark = 0xff
    };

    ProtocolMessageReader();
    ~ProtocolMessageReader();
    void Attach(const common::Packet& packet, unsigned int offset);
    void Clear();
    unsigned long long GetDestination() const;
    unsigned short GetProtocolIdPort() const;
    void GetPayload(void* payload) const;
    void GetPayload(void* payload, unsigned int offset, unsigned int size) const;
    unsigned char ReadU8(unsigned int offset) const;
    unsigned short ReadU16(unsigned int offset) const;
    unsigned int ReadU32(unsigned int offset) const;
    unsigned long long ReadU64(unsigned int offset) const;
    void Trace(unsigned long long value) const;

    unsigned int m_messageSize;
    const unsigned char* m_message;
    ProtocolMessageAccessor::Header m_header;
    bool m_isTerminator;
    common::StationAddress m_stationAddress;
    unsigned char m_sendingPassId;
    unsigned char m_field39;
    bool m_field3a;
};

}
}
}

#endif
