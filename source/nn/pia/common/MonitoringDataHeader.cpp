#include "nn/pia/common/MonitoringDataHeader.hpp"

namespace nn
{
namespace pia
{
namespace common
{

namespace
{
enum
{
    cResultInvalidArgument = 0x10c07
};
}

unsigned int MonitoringDataHeader::GetSerializedSize()
{
    unsigned int size = 0;
    size += sizeof(unsigned char);
    size += sizeof(unsigned char);
    size += sizeof(unsigned char);
    size += sizeof(unsigned char);
    size += sizeof(unsigned short);
    size += sizeof(unsigned char) * cIdentifierSize;
    size += sizeof(unsigned char);
    size += sizeof(unsigned char);
    return size;
}

Result MonitoringDataHeader::Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const
{
    if (buffer == 0)
        return Result(cResultInvalidArgument);
    if (size == 0)
        return Result(cResultInvalidArgument);
    if (maxSize < GetSerializedSize())
        return Result(cResultInvalidArgument);

    unsigned char* cursor = buffer;
    serializeU8(cursor, m_headerType);
    cursor += sizeof(m_headerType);
    serializeU8(cursor, m_dataType);
    cursor += sizeof(m_dataType);
    serializeU8(cursor, m_attribute);
    cursor += sizeof(m_attribute);
    serializeU8(cursor, m_reserved1);
    cursor += sizeof(m_reserved1);
    serializeU16(cursor, m_contentSize);
    cursor += sizeof(m_contentSize);
    for (int index = 0; index < cIdentifierSize; ++index)
    {
        serializeU8(cursor, m_identifier[index]);
        cursor += sizeof(m_identifier[index]);
    }
    serializeU8(cursor, m_keyIndex);
    cursor += sizeof(m_keyIndex);
    serializeU8(cursor, m_reserved2);
    cursor += sizeof(m_reserved2);

    *size = static_cast<unsigned int>(cursor - buffer);
    return 0;
}

Result MonitoringDataHeader::Deserialize(const unsigned char* buffer, unsigned int size)
{
    if (buffer == 0)
        return Result(cResultInvalidArgument);
    if (size < GetSerializedSize())
        return Result(cResultInvalidArgument);

    const unsigned char* cursor = buffer;
    m_headerType = deserializeU8(cursor);
    cursor += sizeof(m_headerType);
    m_dataType = deserializeU8(cursor);
    cursor += sizeof(m_dataType);
    m_attribute = deserializeU8(cursor);
    cursor += sizeof(m_attribute);
    m_reserved1 = deserializeU8(cursor);
    cursor += sizeof(m_reserved1);
    m_contentSize = deserializeU16(cursor);
    cursor += sizeof(m_contentSize);
    for (int index = 0; index < cIdentifierSize; ++index)
    {
        m_identifier[index] = deserializeU8(cursor);
        cursor += sizeof(m_identifier[index]);
    }
    m_keyIndex = deserializeU8(cursor);
    cursor += sizeof(m_keyIndex);
    m_reserved2 = deserializeU8(cursor);
    cursor += sizeof(m_reserved2);

    return 0;
}

void MonitoringDataHeader::Trace(unsigned long long value) const
{
}

}
}
}
