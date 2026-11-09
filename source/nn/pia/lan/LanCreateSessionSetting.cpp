#include "nn/pia/lan/LanCreateSessionSetting.hpp"
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
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cResultBufferTooSmall = 0x10c04
};
}

LanCreateSessionSetting::LanCreateSessionSetting() : session::CreateSessionSetting()
{
    gameMode = 0;
    sessionType = cLanSessionTypeInvalid;
    applicationDataSize = 0;
    openSession = false;
    stateFlags = 0;
    OSBlockSet(attributes, 0, sizeof(attributes));
    OSBlockSet(applicationData, 0, sizeof(applicationData));
}

LanCreateSessionSetting::~LanCreateSessionSetting()
{
}
void LanCreateSessionSetting::SetGameMode(unsigned int value)
{
    gameMode = value;
    stateFlags |= cLanSettingGameMode;
}
unsigned int LanCreateSessionSetting::GetGameMode() const
{
    return gameMode;
}
bool LanCreateSessionSetting::IsSetGameMode() const
{
    return (stateFlags & cLanSettingGameMode) != 0;
}
void LanCreateSessionSetting::SetSessionType(LanSessionType value)
{
    sessionType = value;
    stateFlags |= cLanSettingSessionType;
}
LanSessionType LanCreateSessionSetting::GetSessionType() const
{
    return sessionType;
}
bool LanCreateSessionSetting::IsSetSessionType() const
{
    return (stateFlags & cLanSettingSessionType) != 0;
}
Result LanCreateSessionSetting::SetAttribute(unsigned int index, unsigned int value)
{
    if (index >= cLanAttributeCount)
        return Result(cResultInvalidArgument);
    attributes[index] = value;
    stateFlags |= 1 << (index + cLanAttributeFlagShift);
    return Result(0);
}
unsigned int LanCreateSessionSetting::GetAttribute(unsigned int index) const
{
    return index < cLanAttributeCount ? attributes[index] : 0;
}
bool LanCreateSessionSetting::IsSetAttribute(unsigned int index) const
{
    return (stateFlags >> (index + cLanAttributeFlagShift) & 1) != 0;
}
Result LanCreateSessionSetting::SetApplicationData(const void* data, unsigned int size)
{
    if (data == 0)
        return Result(cResultInvalidArgument);
    if (size > cLanApplicationDataCapacity)
        return Result(cResultInvalidArgument);
    OSBlockMove(applicationData, data, size, FALSE);
    applicationDataSize = size;
    stateFlags |= cLanSettingApplicationData;
    return Result(0);
}
Result LanCreateSessionSetting::GetApplicationData(void* output, unsigned int size) const
{
    if (output == 0)
        return Result(cResultInvalidArgument);
    if (applicationDataSize == 0)
        return Result(cResultInvalidState);
    if (size < applicationDataSize)
        return Result(cResultBufferTooSmall);
    OSBlockMove(output, applicationData, applicationDataSize, FALSE);
    return Result(0);
}
unsigned int LanCreateSessionSetting::GetApplicationDataSize() const
{
    return applicationDataSize;
}
bool LanCreateSessionSetting::IsSetApplicationData() const
{
    return (stateFlags & cLanSettingApplicationData) != 0;
}
void LanCreateSessionSetting::SetOpenSession(bool value)
{
    openSession = value;
    stateFlags |= cLanSettingOpenSession;
}
bool LanCreateSessionSetting::IsOpenSession() const
{
    return openSession;
}
bool LanCreateSessionSetting::IsSetOpenSession() const
{
    return (stateFlags & cLanSettingOpenSession) != 0;
}
void LanCreateSessionSetting::Reset()
{
    session::CreateSessionSetting::Reset();
    gameMode = 0;
    sessionType = cLanSessionTypeInvalid;
    openSession = false;
    OSBlockSet(attributes, 0, sizeof(attributes));
    OSBlockSet(applicationData, 0, sizeof(applicationData));
    applicationDataSize = 0;
    stateFlags = 0;
}
void LanCreateSessionSetting::Trace(unsigned long long value) const
{
}
void LanCreateSessionSetting::Copy(const LanCreateSessionSetting& other)
{
    gameMode = other.gameMode;
    sessionType = other.sessionType;
    OSBlockMove(attributes, other.attributes, sizeof(attributes), FALSE);
    OSBlockMove(applicationData, other.applicationData, sizeof(applicationData), FALSE);
    applicationDataSize = other.applicationDataSize;
    openSession = other.openSession;
    stateFlags = other.stateFlags;
}
LanCreateSessionSetting& LanCreateSessionSetting::operator=(const LanCreateSessionSetting& other)
{
    if (this != &other)
        Copy(other);
    return *this;
}

}
}
}
