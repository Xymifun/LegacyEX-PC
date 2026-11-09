#include "nn/pia/common/MonitoringContentHeader.hpp"

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

unsigned int MonitoringContentHeader::GetSerializedSize()
{
    unsigned int size = 0;
    size += sizeof(unsigned char);
    size += sizeof(unsigned char);
    size += sizeof(unsigned char);
    size += sizeof(unsigned char);
    size += sizeof(unsigned short);
    size += sizeof(unsigned char) * cMetadataSize;
    return size;
}

Result MonitoringContentHeader::Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const
{
    if (buffer == 0)
        return cResultInvalidArgument;
    if (size == 0)
        return cResultInvalidArgument;
    if (maxSize < GetSerializedSize())
        return cResultInvalidArgument;

    unsigned char* cursor = buffer;
    int index;
    serializeU8(cursor, m_contentType);
    cursor += sizeof(m_contentType);
    serializeU8(cursor, m_contentVersion);
    cursor += sizeof(m_contentVersion);
    serializeU8(cursor, m_attribute);
    cursor += sizeof(m_attribute);
    serializeU8(cursor, m_status);
    cursor += sizeof(m_status);
    serializeU16(cursor, m_contentSize);
    cursor += sizeof(m_contentSize);
    for (index = 0; index < cMetadataSize; ++index)
    {
        serializeU8(cursor, m_metadata[index]);
        cursor += sizeof(m_metadata[index]);
    }

    *size = static_cast<unsigned int>(cursor - buffer);
    return 0;
}

}
}
}
