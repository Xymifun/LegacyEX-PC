#include "nn/pia/lan/LanUpdateSessionSetting.hpp"
#include <coreinit/memory.h>

namespace nn
{
namespace pia
{
namespace lan
{

namespace
{
enum
{
    cResultBufferTooSmall = 0x10c04
};
}

LanUpdateSessionSetting::LanUpdateSessionSetting()
    : session::UpdateSessionSetting(),
      m_minParticipantNum(0xffff),
      m_maxParticipantNum(0xffff),
      m_sessionType(cLanSessionTypeInvalid),
      m_applicationDataSize(0)
{
    OSBlockSet(m_attributes, 0, sizeof(m_attributes));
    OSBlockSet(m_applicationData, 0, sizeof(m_applicationData));
}

LanUpdateSessionSetting::~LanUpdateSessionSetting()
{
}
unsigned short LanUpdateSessionSetting::GetMinParticipantNum() const
{
    return m_minParticipantNum;
}
unsigned short LanUpdateSessionSetting::GetMaxParticipantNum() const
{
    return m_maxParticipantNum;
}
LanSessionType LanUpdateSessionSetting::GetSessionType() const
{
    return m_sessionType;
}
unsigned int LanUpdateSessionSetting::GetAttribute(unsigned int index) const
{
    return index < cLanAttributeCount ? m_attributes[index] : 0;
}
Result LanUpdateSessionSetting::SetApplicationData(const void* data, unsigned int size)
{
    if (data == 0)
        return ResultInvalidArgument();
    if (size > sizeof(m_applicationData))
        return ResultInvalidArgument();
    OSBlockSet(m_applicationData, 0, sizeof(m_applicationData));
    OSBlockMove(m_applicationData, data, size, FALSE);
    m_applicationDataSize = size;
    m_modificationFlag |= 1 << cModificationFlag_ApplicationData;
    return Result(0);
}
Result LanUpdateSessionSetting::GetApplicationData(void* output, unsigned int size) const
{
    if (output == 0)
        return ResultInvalidArgument();
    if (m_applicationDataSize == 0)
        return ResultInvalidState();
    if (size < m_applicationDataSize)
        return Result(cResultBufferTooSmall);
    OSBlockMove(output, m_applicationData, m_applicationDataSize, FALSE);
    return Result(0);
}
unsigned int LanUpdateSessionSetting::GetApplicationDataSize() const
{
    return m_applicationDataSize;
}
void LanUpdateSessionSetting::Reset()
{
    session::UpdateSessionSetting::Reset();
    m_minParticipantNum = 0xffff;
    m_maxParticipantNum = 0xffff;
    m_sessionType = cLanSessionTypeInvalid;
    OSBlockSet(m_attributes, 0, sizeof(m_attributes));
    OSBlockSet(m_applicationData, 0, sizeof(m_applicationData));
    m_applicationDataSize = 0;
}
void LanUpdateSessionSetting::Trace(unsigned long long) const
{
}

}
}
}
