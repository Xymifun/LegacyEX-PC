#include "nn/pia/transport/ProtocolMessageWriter.hpp"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/transport/ProtocolMessageAccessor.hpp"

#include <coreinit/memory.h>

namespace nn
{
namespace pia
{
namespace transport
{

ProtocolMessageWriter::ProtocolMessageWriter() : m_source(0), m_protocolId(ProtocolId::INVALID)
{
}

ProtocolMessageWriter::~ProtocolMessageWriter()
{
}

void ProtocolMessageWriter::SetSource(unsigned long long source)
{
    m_source = source;
}

void ProtocolMessageWriter::Reset(const ProtocolId& protocolId, unsigned int messageSize, bool isReliable,
                                  bool isPriority)
{
    m_messageSize = messageSize;
    m_protocolId = protocolId;
    m_isReliable = isReliable;
    m_isPriority = isPriority;
    OSBlockSet(m_field12, 0, sizeof(m_field12));
    m_priority = 0;
    m_messageBufferNum = 0;
}

void ProtocolMessageWriter::AddMessageBuffer(common::Packet* packet, void* buffer,
                                             unsigned long long destinationStationId, bool field10, bool field11)
{
    MessageBuffer& messageBuffer = m_messageBuffers[m_messageBufferNum];
    messageBuffer.m_packet = packet;
    messageBuffer.m_buffer = buffer;
    messageBuffer.m_destinationStationId = destinationStationId;
    messageBuffer.m_field10 = field10;
    messageBuffer.m_field11 = field11;
    ++m_messageBufferNum;
}

void ProtocolMessageWriter::Commit()
{
    MessageBuffer& firstBuffer = m_messageBuffers[0];
    ProtocolMessageAccessor::Header header;
    header.m_flags = static_cast<unsigned char>(firstBuffer.m_field10 | (firstBuffer.m_field11 ? 2 : 0)
                                                | (m_isReliable ? 4 : 0) | (m_isPriority ? 8 : 0));
    header.m_messageSize = static_cast<unsigned short>(m_messageSize);
    header.m_destination = firstBuffer.m_destinationStationId;
    header.m_source = m_source;
    header.m_protocolId = m_protocolId;
    OSBlockMove(header.m_field1a, m_field12, sizeof(header.m_field1a), FALSE);
    unsigned int serializedSize;
    header.Serialize(static_cast<unsigned char*>(firstBuffer.m_buffer), &serializedSize);

    const unsigned int copySize = ((m_messageSize + 23U) & ~3U) + 4U;
    for (unsigned int index = 1; index < m_messageBufferNum; ++index)
    {
        MessageBuffer& messageBuffer = m_messageBuffers[index];
        OSBlockMove(messageBuffer.m_buffer, firstBuffer.m_buffer, copySize, FALSE);
        header.m_flags = static_cast<unsigned char>(messageBuffer.m_field10 | (messageBuffer.m_field11 ? 2 : 0)
                                                    | (m_isReliable ? 4 : 0) | (m_isPriority ? 8 : 0));
        header.m_destination = messageBuffer.m_destinationStationId;
        header.Serialize(static_cast<unsigned char*>(messageBuffer.m_buffer), &serializedSize);
    }

    if (m_priority != 0)
    {
        for (unsigned int index = 0; index < m_messageBufferNum; ++index)
        {
            common::Packet* packet = m_messageBuffers[index].m_packet;
            if (m_priority > packet->m_field600)
                packet->m_field600 = m_priority;
        }
    }
}

void ProtocolMessageWriter::SetPayload(const void* payload)
{
    OSBlockMove(static_cast<unsigned char*>(m_messageBuffers[0].m_buffer) + cMessageHeaderSize, payload, m_messageSize,
                FALSE);
}

void ProtocolMessageWriter::SetPayload(const void* payload, unsigned int offset, unsigned int size)
{
    OSBlockMove(static_cast<unsigned char*>(m_messageBuffers[0].m_buffer) + cMessageHeaderSize + offset, payload, size,
                FALSE);
}

void ProtocolMessageWriter::WriteU8(unsigned char value, unsigned int offset)
{
    unsigned char* buffer = static_cast<unsigned char*>(m_messageBuffers[0].m_buffer) + cMessageHeaderSize + offset;
    buffer[0] = value;
}

void ProtocolMessageWriter::WriteU32(unsigned int value, unsigned int offset)
{
    unsigned char* buffer = static_cast<unsigned char*>(m_messageBuffers[0].m_buffer);
    unsigned char* cursor = buffer + offset;
    cursor += cMessageHeaderSize;
    cursor[0] = static_cast<unsigned char>((value >> 24) & 0xff);
    cursor[1] = static_cast<unsigned char>((value >> 16) & 0xff);
    cursor[2] = static_cast<unsigned char>((value >> 8) & 0xff);
    cursor[3] = static_cast<unsigned char>(value & 0xff);
}

void ProtocolMessageWriter::WriteU64(unsigned long long value, unsigned int offset)
{
    unsigned char* buffer = static_cast<unsigned char*>(m_messageBuffers[0].m_buffer);
    unsigned char* cursor = buffer + offset;
    cursor += cMessageHeaderSize;
    cursor[0] = static_cast<unsigned char>((value >> 56) & 0xff);
    cursor[1] = static_cast<unsigned char>((value >> 48) & 0xff);
    cursor[2] = static_cast<unsigned char>((value >> 40) & 0xff);
    cursor[3] = static_cast<unsigned char>((value >> 32) & 0xff);
    cursor[4] = static_cast<unsigned char>((value >> 24) & 0xff);
    cursor[5] = static_cast<unsigned char>((value >> 16) & 0xff);
    cursor[6] = static_cast<unsigned char>((value >> 8) & 0xff);
    cursor[7] = static_cast<unsigned char>(value & 0xff);
}

}
}
}
