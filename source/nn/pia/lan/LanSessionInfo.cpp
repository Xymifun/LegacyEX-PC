#include "nn/pia/lan/LanSessionInfo.hpp"

#include "nn/pia/String.hpp"
#include "platform/wiiu/pead.hpp"

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
    cResultBufferTooSmall = 0x10c04,
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08
};
}

LanSessionInfo::LanSessionInfo()
    : m_gameMode(0),
      m_sessionId(0),
      m_currentParticipantNum(0),
      m_minParticipantNum(0),
      m_maxParticipantNum(0),
      m_open(false),
      m_systemCommunicationVersion(5),
      m_applicationCommunicationVersion(0),
      m_sessionType(0)
{
    OSBlockSet(m_applicationData, 0, sizeof(m_applicationData));
    m_applicationDataSize = 0;
    OSBlockSet(m_attribute, 0, sizeof(m_attribute));
    OSBlockSet(m_sessionParam, 0, sizeof(m_sessionParam));
    transport::StationConnectionInfo stationConnectionInfo;
    m_stationConnectionInfo = stationConnectionInfo;
    for (unsigned int index = 0; index < cMaxStationInfo; ++index)
        m_stationInfo[index].Clear();
    m_stationInfoCount = 0;
}

LanSessionInfo::LanSessionInfo(const LanSessionInfo& other)
{
    Copy(other);
}

LanSessionInfo::~LanSessionInfo()
{
}

LanSessionInfo& LanSessionInfo::operator=(const LanSessionInfo& other)
{
    Copy(other);
    return *this;
}

unsigned int LanSessionInfo::GetGameMode() const
{
    return m_gameMode;
}

unsigned int LanSessionInfo::GetSessionId() const
{
    return m_sessionId;
}

unsigned short LanSessionInfo::GetCurrentParticipantNum() const
{
    return m_currentParticipantNum;
}

unsigned short LanSessionInfo::GetMinParticipantNum() const
{
    return m_minParticipantNum;
}

unsigned short LanSessionInfo::GetMaxParticipantNum() const
{
    return m_maxParticipantNum;
}

bool LanSessionInfo::IsOpened() const
{
    return m_open;
}

Result LanSessionInfo::GetApplicationData(void* buffer, unsigned int bufferSize) const
{
    if (buffer == 0)
    {
        Result result(cResultInvalidArgument);
        return result;
    }
    else if (bufferSize < m_applicationDataSize)
    {
        Result result(cResultBufferTooSmall);
        return result;
    }
    else
    {
        OSBlockMove(buffer, m_applicationData, m_applicationDataSize, FALSE);
        return 0;
    }
}

unsigned int LanSessionInfo::GetApplicationDataSize() const
{
    return m_applicationDataSize;
}

void LanSessionInfo::Clear()
{
    m_gameMode = 0;
    m_sessionId = 0;
    m_currentParticipantNum = 0;
    m_minParticipantNum = 0;
    m_maxParticipantNum = 0;
    m_open = false;
    OSBlockSet(m_attribute, 0, sizeof(m_attribute));
    OSBlockSet(m_sessionParam, 0, sizeof(m_sessionParam));
    OSBlockSet(m_applicationData, 0, sizeof(m_applicationData));
    m_applicationDataSize = 0;
    m_systemCommunicationVersion = 5;
    m_applicationCommunicationVersion = 0;
    m_sessionType = 0;
    transport::StationConnectionInfo stationConnectionInfo;
    m_stationConnectionInfo = stationConnectionInfo;
    for (unsigned int index = 0; index < cMaxStationInfo; ++index)
        m_stationInfo[index].Clear();
    m_stationInfoCount = 0;
}

void LanSessionInfo::Trace(unsigned long long) const
{
}

Result LanSessionInfo::GetAttribute(unsigned char index, unsigned int* value) const
{
    if (value == 0)
    {
        return Result(cResultInvalidArgument);
    }
    else if (index >= cAttributeCount)
    {
        Result result(cResultInvalidArgument);
        return result;
    }
    else
    {
        *value = m_attribute[index];
        return Result(0);
    }
}

unsigned short LanSessionInfo::GetSessionType() const
{
    return m_sessionType;
}

unsigned long long LanSessionInfo::GetHostPrincipalId() const
{
    return m_stationConnectionInfo.m_stationLocation.m_principalId;
}

Result LanSessionInfo::GetSessionStationInfo(LanStationInfo* stationInfo, unsigned int count)
{
    if (stationInfo == 0)
        return cResultInvalidArgument;
    if (m_stationInfoCount == 0)
        return cResultInvalidState;
    if (m_stationInfoCount > count)
        return cResultBufferTooSmall;

    for (unsigned int index = 0; index < m_stationInfoCount; ++index)
    {
        stationInfo[index].SetRole(m_stationInfo[index].GetRole());
        stationInfo[index].SetUserName(m_stationInfo[index].GetUserName());
        stationInfo[index].SetStationId(m_stationInfo[index].GetStationId());
    }
    return 0;
}

unsigned int LanSessionInfo::GetSessionStationInfoSize() const
{
    return m_stationInfoCount;
}

unsigned char LanSessionInfo::GetApplicationCommunicationVersion() const
{
    return m_applicationCommunicationVersion;
}

unsigned char LanSessionInfo::GetSystemCommunicationVersion() const
{
    return m_systemCommunicationVersion;
}

void LanSessionInfo::Copy(const LanSessionInfo& other)
{
    m_gameMode = other.m_gameMode;
    m_sessionId = other.m_sessionId;
    m_currentParticipantNum = other.m_currentParticipantNum;
    m_minParticipantNum = other.m_minParticipantNum;
    m_maxParticipantNum = other.m_maxParticipantNum;
    m_open = other.m_open;
    OSBlockMove(m_attribute, other.m_attribute, sizeof(m_attribute), FALSE);
    OSBlockMove(m_applicationData, other.m_applicationData, sizeof(m_applicationData), FALSE);
    OSBlockMove(m_sessionParam, other.m_sessionParam, sizeof(m_sessionParam), FALSE);
    m_applicationDataSize = other.m_applicationDataSize;
    m_sessionType = other.m_sessionType;
    m_systemCommunicationVersion = other.m_systemCommunicationVersion;
    m_applicationCommunicationVersion = other.m_applicationCommunicationVersion;
    m_stationConnectionInfo = other.m_stationConnectionInfo;
    m_stationInfoCount = other.m_stationInfoCount;

    for (unsigned int index = 0; index < cMaxStationInfo; ++index)
    {
        m_stationInfo[index].SetUserName(other.m_stationInfo[index].GetUserName());
        m_stationInfo[index].SetRole(other.m_stationInfo[index].GetRole());
        m_stationInfo[index].SetStationId(other.m_stationInfo[index].GetStationId());
    }
}

void LanSessionInfo::SetGameMode(unsigned int gameMode)
{
    m_gameMode = gameMode;
}

void LanSessionInfo::SetSessionId(unsigned int sessionId)
{
    m_sessionId = sessionId;
}

void LanSessionInfo::SetCurrentParticipantNum(unsigned short participantNum)
{
    m_currentParticipantNum = participantNum;
}

void LanSessionInfo::SetMinParticipantNum(unsigned short participantNum)
{
    m_minParticipantNum = participantNum;
}

void LanSessionInfo::SetMaxParticipantNum(unsigned short participantNum)
{
    m_maxParticipantNum = participantNum;
}

void LanSessionInfo::SetOpen(bool open)
{
    m_open = open;
}

void LanSessionInfo::SetSessionType(LanSessionType sessionType)
{
    m_sessionType = sessionType;
}

void LanSessionInfo::SetAttribute(unsigned char index, unsigned int value)
{
    m_attribute[index] = value;
}

void LanSessionInfo::SetApplicationData(const void* data, unsigned int size)
{
    unsigned int copySize = size;
    if (copySize > cApplicationDataMaxSize)
        copySize = cApplicationDataMaxSize;

    OSBlockSet(m_applicationData, 0, sizeof(m_applicationData));
    OSBlockMove(m_applicationData, data, copySize, FALSE);
    m_applicationDataSize = copySize;
}

transport::StationConnectionInfo& LanSessionInfo::GetStationConnectionInfo()
{
    return m_stationConnectionInfo;
}

void LanSessionInfo::SetStationConnectionInfo(const transport::StationConnectionInfo& info)
{
    m_stationConnectionInfo = info;
}

void LanSessionInfo::AddSessionStationInfo(const LanStationInfo& stationInfo)
{
    if (m_stationInfoCount < cMaxStationInfo)
    {
        m_stationInfo[m_stationInfoCount].SetRole(stationInfo.GetRole());
        m_stationInfo[m_stationInfoCount].SetUserName(stationInfo.GetUserName());
        m_stationInfo[m_stationInfoCount].SetStationId(stationInfo.GetStationId());
        ++m_stationInfoCount;
    }
}

void LanSessionInfo::ClearSessionStationInfo()
{
    for (unsigned int index = 0; index < cMaxStationInfo; ++index)
        m_stationInfo[index].Clear();
    m_stationInfoCount = 0;
}

void LanSessionInfo::SetSystemCommunicationVersion(unsigned char version)
{
    m_systemCommunicationVersion = version;
}

void LanSessionInfo::SetApplicationCommunicationVersion(unsigned char version)
{
    m_applicationCommunicationVersion = version;
}

const unsigned char* LanSessionInfo::GetSessionParam() const
{
    return m_sessionParam;
}

namespace
{
enum
{
    cSerializedFixedSize = 0x1cf,
    cUserNameAsciiBytes = 0x28,
    cUserNameUtf16Units = 0x14,
    cSessionParamSize = 0x20,
    cApplicationDataSize = 0x180,
    cAttributeCount = 6
};

const unsigned long long cTraceSessionInfo = 0x0100000000000000ULL;
}

unsigned int LanSessionInfo::GetSerializedSize() const
{
    return (m_stationConnectionInfo.GetSerializedSize() >> 1) + cSerializedFixedSize
           + LanStationInfo::GetSerializedSize() * cMaxStationInfo;
}

Result LanSessionInfo::Serialize(unsigned char* buffer, unsigned int* size, unsigned int bufferSize) const
{
    if (buffer == 0)
        return cResultInvalidArgument;
    if (size == 0)
        return cResultInvalidArgument;
    if (bufferSize < GetSerializedSize())
        return cResultInvalidArgument;

    unsigned char* cursor = buffer;
    common::serializeU32(cursor, m_gameMode);
    cursor += 4;
    common::serializeU32(cursor, m_sessionId);
    cursor += 4;
    for (unsigned int index = 0; index < cAttributeCount; ++index)
    {
        common::serializeU32(cursor, m_attribute[index]);
        cursor += 4;
    }
    common::serializeU16(cursor, m_currentParticipantNum);
    cursor += 2;
    common::serializeU16(cursor, m_minParticipantNum);
    cursor += 2;
    common::serializeU16(cursor, m_maxParticipantNum);
    cursor += 2;
    common::serializeU8(cursor, m_systemCommunicationVersion);
    cursor += 1;
    common::serializeU8(cursor, m_applicationCommunicationVersion);
    cursor += 1;
    common::serializeU16(cursor, m_sessionType);
    cursor += 2;
    for (unsigned int index = 0; index < cApplicationDataSize; ++index)
    {
        common::serializeU8(cursor, m_applicationData[index]);
        cursor += 1;
    }
    common::serializeU32(cursor, m_applicationDataSize);
    cursor += 4;
    common::serializeU8(cursor, m_open);
    cursor += 1;

    unsigned int written;
    m_stationConnectionInfo.m_stationLocation.m_stationAddress.Serialize(cursor, &written,
                                                                         bufferSize - (cursor - buffer));
    cursor += m_stationConnectionInfo.m_stationLocation.m_stationAddress.GetSerializedSize();
    common::serializeU64(cursor, m_stationConnectionInfo.m_stationLocation.m_principalId);
    cursor += 8;
    common::serializeU32(cursor, m_stationConnectionInfo.m_stationLocation.m_field18);
    cursor += 4;
    common::serializeU32(cursor, m_stationConnectionInfo.m_stationLocation.m_field1c);
    cursor += 4;
    common::serializeU8(cursor, m_stationConnectionInfo.m_stationLocation.m_field20);
    cursor += 1;
    common::serializeU8(cursor, m_stationConnectionInfo.m_stationLocation.m_field21);
    cursor += 1;
    common::serializeU8(cursor, m_stationConnectionInfo.m_stationLocation.m_field22);
    cursor += 1;
    common::serializeU8(cursor, m_stationConnectionInfo.m_stationLocation.m_field23);
    cursor += 1;
    common::serializeU8(cursor, m_stationConnectionInfo.m_stationLocation.m_field24);
    cursor += 1;
    common::serializeU8(cursor, m_stationConnectionInfo.m_stationLocation.m_field25);
    cursor += 1;
    common::serializeU8(cursor, m_stationConnectionInfo.m_stationLocation.m_field26);
    cursor += 1;
    m_stationConnectionInfo.m_stationLocation.m_inetAddress.Serialize(cursor, &written, bufferSize - (cursor - buffer));
    cursor += 6;

    for (unsigned int index = 0; index < cMaxStationInfo; ++index)
    {
        if (index < m_stationInfoCount)
        {
            const LanStationInfo& stationInfo = m_stationInfo[index];
            stationInfo.GetUserName().Trace(cTraceSessionInfo);
            common::serializeU8(cursor, stationInfo.GetRole());
            cursor += 1;
            common::serializeU8(cursor, stationInfo.GetUserName().GetEncodingType());
            cursor += 1;
            if (stationInfo.GetUserName().GetEncodingType() == common::String::EncodingType_Utf16)
            {
                const unsigned short* name = static_cast<const unsigned short*>(stationInfo.GetUserName().GetBuffer());
                for (unsigned int unit = 0; unit < cUserNameUtf16Units; ++unit)
                {
                    common::serializeU16(cursor, name[unit]);
                    cursor += 2;
                }
            }
            else
            {
                const char* name = stationInfo.GetUserName().GetCharArray();
                for (unsigned int unit = 0; unit < cUserNameAsciiBytes; ++unit)
                {
                    common::serializeU8(cursor, name[unit]);
                    cursor += 1;
                }
            }
            common::serializeU64(cursor, stationInfo.GetStationId());
            cursor += 8;
        }
        else
        {
            for (unsigned int offset = 0; offset < LanStationInfo::GetSerializedSize(); ++offset)
            {
                common::serializeU8(cursor, 0);
                cursor += 1;
            }
        }
    }

    for (unsigned int index = 0; index < cSessionParamSize; ++index)
    {
        common::serializeU8(cursor, m_sessionParam[index]);
        cursor += 1;
    }
    *size = cursor - buffer;
    return 0;
}

Result LanSessionInfo::Deserialize(const unsigned char* buffer, unsigned int bufferSize)
{
    if (buffer == 0)
        return cResultInvalidArgument;
    if (bufferSize < GetSerializedSize())
        return cResultInvalidArgument;

    const unsigned char* cursor = buffer;
    m_gameMode = common::deserializeU32(cursor);
    cursor += 4;
    m_sessionId = common::deserializeU32(cursor);
    cursor += 4;
    for (unsigned int index = 0; index < cAttributeCount; ++index)
    {
        m_attribute[index] = common::deserializeU32(cursor);
        cursor += 4;
    }
    m_currentParticipantNum = common::deserializeU16(cursor);
    cursor += 2;
    m_minParticipantNum = common::deserializeU16(cursor);
    cursor += 2;
    m_maxParticipantNum = common::deserializeU16(cursor);
    cursor += 2;
    m_systemCommunicationVersion = common::deserializeU8(cursor);
    cursor += 1;
    m_applicationCommunicationVersion = common::deserializeU8(cursor);
    cursor += 1;
    m_sessionType = common::deserializeU16(cursor);
    cursor += 2;
    for (unsigned int index = 0; index < cApplicationDataSize; ++index)
    {
        m_applicationData[index] = common::deserializeU8(cursor);
        cursor += 1;
    }
    m_applicationDataSize = common::deserializeU32(cursor);
    cursor += 4;
    m_open = common::deserializeU8(cursor) != 0;
    cursor += 1;

    m_stationConnectionInfo.m_stationLocation.m_stationAddress.Deserialize(cursor);
    cursor += m_stationConnectionInfo.m_stationLocation.m_stationAddress.GetSerializedSize();
    m_stationConnectionInfo.m_stationLocation.m_principalId = common::deserializeU64(cursor);
    cursor += 8;
    m_stationConnectionInfo.m_stationLocation.m_field18 = common::deserializeU32(cursor);
    cursor += 4;
    m_stationConnectionInfo.m_stationLocation.m_field1c = common::deserializeU32(cursor);
    cursor += 4;
    m_stationConnectionInfo.m_stationLocation.m_field20 = common::deserializeU8(cursor);
    cursor += 1;
    m_stationConnectionInfo.m_stationLocation.m_field21 = common::deserializeU8(cursor);
    cursor += 1;
    m_stationConnectionInfo.m_stationLocation.m_field22 = common::deserializeU8(cursor);
    cursor += 1;
    m_stationConnectionInfo.m_stationLocation.m_field23 = common::deserializeU8(cursor);
    cursor += 1;
    m_stationConnectionInfo.m_stationLocation.m_field24 = common::deserializeU8(cursor);
    cursor += 1;
    m_stationConnectionInfo.m_stationLocation.m_field25 = common::deserializeU8(cursor);
    cursor += 1;
    m_stationConnectionInfo.m_stationLocation.m_field26 = common::deserializeU8(cursor);
    cursor += 1;
    m_stationConnectionInfo.m_stationLocation.m_inetAddress.Deserialize(cursor);
    cursor += 6;
    m_stationConnectionInfo.m_localStationLocation.SetStationLocation(m_stationConnectionInfo.m_stationLocation);

    m_stationInfoCount = 0;
    for (unsigned int index = 0; index < cMaxStationInfo; ++index)
    {
        m_stationInfo[index].SetRole(
            static_cast<session::SessionStationInfo::StationInfoRole>(common::deserializeU8(cursor)));
        cursor += 1;
        if (common::deserializeU8(cursor) == common::String::EncodingType_Utf16)
        {
            cursor += 1;
            unsigned short name[cUserNameUtf16Units];
            for (unsigned int unit = 0; unit < cUserNameUtf16Units; ++unit)
            {
                name[unit] = common::deserializeU16(cursor);
                cursor += 2;
            }
            common::FixedString<10> userName;
            userName.SetString(name, userName.CalcBytes(name, common::String::EncodingType_Utf16),
                               common::String::EncodingType_Utf16);
            m_stationInfo[index].SetUserName(userName);
        }
        else
        {
            cursor += 1;
            char name[cUserNameAsciiBytes];
            for (unsigned int unit = 0; unit < cUserNameAsciiBytes; ++unit)
            {
                name[unit] = common::deserializeU8(cursor);
                cursor += 1;
            }
            common::FixedString<10> userName;
            userName.SetString(name, userName.CalcBytes(name, common::String::EncodingType_Ascii),
                               common::String::EncodingType_Ascii);
            m_stationInfo[index].SetUserName(userName);
        }
        m_stationInfo[index].SetStationId(common::deserializeU64(cursor));
        cursor += 8;
        m_stationInfo[index].GetUserName().Trace(cTraceSessionInfo);
        if (m_stationInfo[index].GetRole() != session::SessionStationInfo::cStationInfoRole_Invalid
            || m_stationInfo[index].GetUserName().GetStringLength() != 0)
            ++m_stationInfoCount;
    }

    for (unsigned int index = 0; index < cSessionParamSize; ++index)
    {
        m_sessionParam[index] = common::deserializeU8(cursor);
        cursor += 1;
    }
    return 0;
}

}
}
}
