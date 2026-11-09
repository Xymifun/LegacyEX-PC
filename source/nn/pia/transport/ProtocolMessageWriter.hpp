#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PROTOCOL_MESSAGE_WRITER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PROTOCOL_MESSAGE_WRITER_HPP

#include "nn/pia/Reconstruction.hpp"

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

class ProtocolMessageWriter
{
public:
    enum
    {
        cMessageBufferNumMax = 31,
        cMessageHeaderSize = 24
    };

    class MessageBuffer
    {
    public:
        common::Packet* m_packet;
        void* m_buffer;
        unsigned long long m_destinationStationId;
        bool m_field10;
        bool m_field11;
    };

    ProtocolMessageWriter();
    ~ProtocolMessageWriter();
    void SetSource(unsigned long long source);
    void Reset(const ProtocolId& protocolId, unsigned int messageSize, bool isReliable, bool isPriority);
    void Commit();
    void AddMessageBuffer(common::Packet* packet, void* buffer, unsigned long long destinationStationId, bool field10,
                          bool field11);
    void SetPayload(const void* payload);
    void SetPayload(const void* payload, unsigned int offset, unsigned int size);
    void WriteU8(unsigned char value, unsigned int offset);
    void WriteU16(unsigned short value, unsigned int offset);
    void WriteU32(unsigned int value, unsigned int offset);
    void WriteU64(unsigned long long value, unsigned int offset);
    void Trace(unsigned long long value) const;

    unsigned int m_messageSize;
    unsigned long long m_source;
    ProtocolId m_protocolId;
    unsigned char m_field12[3];
    bool m_isReliable;
    bool m_isPriority;
    MessageBuffer m_messageBuffers[cMessageBufferNumMax];
    unsigned int m_messageBufferNum;
    unsigned char m_priority;
};

}
}
}

#endif
