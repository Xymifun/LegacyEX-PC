#include "nn/pia/transport/ProtocolMessageReader.hpp"

#include "nn/pia/common/Packet.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

ProtocolMessageReader::ProtocolMessageReader() : m_message(0), m_header(), m_isTerminator(false)
{
}

ProtocolMessageReader::~ProtocolMessageReader()
{
}

void ProtocolMessageReader::Attach(const common::Packet& packet, unsigned int offset)
{
    const unsigned int payloadSize = packet.m_size - common::Packet::cHeaderSize;
    if (offset >= payloadSize)
    {
        m_message = 0;
        m_isTerminator = false;
        return;
    }

    if (*packet.m_packetData.GetPayload(offset) == cTerminatorMark)
    {
        m_message = 0;
        m_isTerminator = true;
        return;
    }

    m_isTerminator = false;
    if (offset + cMessageHeaderSize <= payloadSize)
    {
        const unsigned char* message = packet.m_packetData.GetPayload(offset);
        m_header.Deserialize(message);
        m_messageSize = m_header.m_messageSize;

        if (offset + m_header.m_messageSize + cMessageHeaderSize <= payloadSize)
        {
            m_message = message;
            if ((m_header.m_flags & 4) == 0)
            {
                m_stationAddress = packet.m_field608;
                m_sendingPassId = packet.m_packetData.m_header.m_passId;
                m_field39 = packet.m_field600;
                m_field3a = packet.m_field603;
            }
            else
            {
                m_stationAddress.Clear();
                m_sendingPassId = 0;
                m_field39 = 0;
                m_field3a = packet.m_field603;
            }
            return;
        }
    }

    m_message = 0;
}

void ProtocolMessageReader::Clear()
{
    m_message = 0;
    m_stationAddress.Clear();
    m_sendingPassId = 0;
    m_messageSize = 0;
}

unsigned long long ProtocolMessageReader::GetDestination() const
{
    return m_header.m_destination;
}

unsigned short ProtocolMessageReader::GetProtocolIdPort() const
{
    return ProtocolId(m_header.m_protocolId).GetPort();
}

void ProtocolMessageReader::Trace(unsigned long long value) const
{
}

}
}
}
