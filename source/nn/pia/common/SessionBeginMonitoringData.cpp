#include "nn/pia/common/SessionBeginMonitoringData.hpp"
#include "nn/pia/common/MonitoringDataSender.hpp"
#include "platform/wiiu/pead.hpp"

#include <coreinit/memory.h>

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
    cResultInvalidArgument = 0x10c07,
    cHeaderType = 0x12
};
}

unsigned int SessionBeginMonitoringData::GetSerializedSize()
{
    return MonitoringDataHeader::GetSerializedSize() + SessionBeginMonitoringContent::GetSerializedSize();
}

void SessionBeginMonitoringData::Initialize()
{
    OSBlockSet(this, 0xff, sizeof(MonitoringDataHeader));
    m_headerType = cHeaderType;
    m_dataType = MonitoringDataSender::cDataTypeSessionBegin;
    m_contentSize = static_cast<unsigned short>(GetSerializedSize());
    pead::Random random;
    unsigned long long identifier = random.getU64();
    OSBlockMove(m_identifier, &identifier, sizeof(identifier), false);
    m_keyIndex = static_cast<unsigned char>(random.getU32());
    m_content.Initialize();
}

Result SessionBeginMonitoringData::Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const
{
    const Result invalid = cResultInvalidArgument;
    if (buffer == 0)
        return invalid;
    if (size == 0)
        return invalid;
    if (maxSize < GetSerializedSize())
        return invalid;
    unsigned char* cursor = buffer;
    unsigned int written = 0;
    Result result;
    result = MonitoringDataHeader::Serialize(cursor, &written, maxSize);
    if (result != 0)
        return result;
    cursor += written;
    result = m_content.Serialize(cursor, &written, maxSize - written);
    if (result != 0)
        return result;
    cursor += written;
    *size = static_cast<unsigned int>(cursor - buffer);
    return 0;
}

}
}
}
