#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PROTOCOL_MESSAGE_ACCESSOR_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PROTOCOL_MESSAGE_ACCESSOR_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/common/ISerializable.hpp"

#include <coreinit/memory.h>

namespace nn
{
namespace pia
{
namespace transport
{

class ProtocolMessageAccessor
{
public:
    class Header : public common::ISerializable
    {
    public:
        enum
        {
            cSerializedSize = 24,
            cResultInvalidArgument = 0x10c07
        };

        virtual ~Header() {}

        virtual Result Serialize(unsigned char* buffer, unsigned int* size) const
        {
            if (buffer == 0)
                return cResultInvalidArgument;
            if (size == 0)
                return cResultInvalidArgument;

            unsigned char* cursor = buffer;
            *cursor++ = m_flags;
            const unsigned short messageSize = m_messageSize;
            cursor[0] = static_cast<unsigned char>((messageSize >> 8) & 0xff);
            cursor[1] = static_cast<unsigned char>(messageSize & 0xff);
            cursor += sizeof(m_messageSize);
            const unsigned long long destination = m_destination;
            cursor[0] = static_cast<unsigned char>((destination >> 56) & 0xff);
            cursor[1] = static_cast<unsigned char>((destination >> 48) & 0xff);
            cursor[2] = static_cast<unsigned char>((destination >> 40) & 0xff);
            cursor[3] = static_cast<unsigned char>((destination >> 32) & 0xff);
            cursor[4] = static_cast<unsigned char>((destination >> 24) & 0xff);
            cursor[5] = static_cast<unsigned char>((destination >> 16) & 0xff);
            cursor[6] = static_cast<unsigned char>((destination >> 8) & 0xff);
            cursor[7] = static_cast<unsigned char>(destination & 0xff);
            cursor += sizeof(m_destination);
            const unsigned long long source = m_source;
            cursor[0] = static_cast<unsigned char>((source >> 56) & 0xff);
            cursor[1] = static_cast<unsigned char>((source >> 48) & 0xff);
            cursor[2] = static_cast<unsigned char>((source >> 40) & 0xff);
            cursor[3] = static_cast<unsigned char>((source >> 32) & 0xff);
            cursor[4] = static_cast<unsigned char>((source >> 24) & 0xff);
            cursor[5] = static_cast<unsigned char>((source >> 16) & 0xff);
            cursor[6] = static_cast<unsigned char>((source >> 8) & 0xff);
            cursor[7] = static_cast<unsigned char>(source & 0xff);
            cursor += sizeof(m_source);
            const unsigned short protocolId = m_protocolId;
            cursor[0] = static_cast<unsigned char>((protocolId >> 8) & 0xff);
            cursor[1] = static_cast<unsigned char>(protocolId & 0xff);
            cursor += sizeof(m_protocolId);
            OSBlockMove(cursor, m_field1a, sizeof(m_field1a), FALSE);
            cursor += sizeof(m_field1a);

            *size = static_cast<unsigned int>(cursor - buffer);
            return 0;
        }

        virtual Result Deserialize(const unsigned char* buffer)
        {
            const Result invalid = cResultInvalidArgument;
            if (buffer == 0)
                return invalid;

            const unsigned char* cursor = buffer;
            m_flags = *cursor++;
            m_messageSize = static_cast<unsigned short>(cursor[0] << 8 | cursor[1]);
            cursor += sizeof(m_messageSize);
            unsigned long long destination = 0;
            destination |= static_cast<unsigned long long>(cursor[0]) << 56;
            destination |= static_cast<unsigned long long>(cursor[1]) << 48;
            destination |= static_cast<unsigned long long>(cursor[2]) << 40;
            destination |= static_cast<unsigned long long>(cursor[3]) << 32;
            destination |= static_cast<unsigned long long>(cursor[4]) << 24;
            destination |= static_cast<unsigned long long>(cursor[5]) << 16;
            destination |= static_cast<unsigned long long>(cursor[6]) << 8;
            destination |= static_cast<unsigned long long>(cursor[7]);
            m_destination = destination;
            cursor += sizeof(m_destination);
            unsigned long long source = 0;
            source |= static_cast<unsigned long long>(cursor[0]) << 56;
            source |= static_cast<unsigned long long>(cursor[1]) << 48;
            source |= static_cast<unsigned long long>(cursor[2]) << 40;
            source |= static_cast<unsigned long long>(cursor[3]) << 32;
            source |= static_cast<unsigned long long>(cursor[4]) << 24;
            source |= static_cast<unsigned long long>(cursor[5]) << 16;
            source |= static_cast<unsigned long long>(cursor[6]) << 8;
            source |= static_cast<unsigned long long>(cursor[7]);
            m_source = source;
            cursor += sizeof(m_source);
            m_protocolId = static_cast<unsigned short>(cursor[0] << 8 | cursor[1]);
            cursor += sizeof(m_protocolId);
            OSBlockMove(m_field1a, cursor, sizeof(m_field1a), FALSE);

            return 0;
        }

        virtual unsigned int GetSerializedSize() const { return cSerializedSize; }

        unsigned char m_flags;
        unsigned short m_messageSize;
        unsigned long long m_destination;
        unsigned long long m_source;
        unsigned short m_protocolId;
        unsigned char m_field1a[3];
    };
};

}
}
}

#endif
