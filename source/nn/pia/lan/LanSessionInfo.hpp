#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANSESSIONINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANSESSIONINFO_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/lan/LanCreateSessionSetting.hpp"
#include "nn/pia/lan/LanStationInfo.hpp"
#include "nn/pia/session/ISessionInfo.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanSessionInfo : public session::ISessionInfo
{
public:
    enum
    {
        cAttributeCount = 6,
        cSessionParamSize = 32,
        cApplicationDataMaxSize = 384,
        cMaxStationInfo = 16
    };

    LanSessionInfo();
    LanSessionInfo(const LanSessionInfo& other);
    virtual ~LanSessionInfo();
    LanSessionInfo& operator=(const LanSessionInfo& other);

    virtual unsigned int GetGameMode() const;
    virtual unsigned int GetSessionId() const;
    virtual unsigned short GetCurrentParticipantNum() const;
    virtual unsigned short GetMinParticipantNum() const;
    virtual unsigned short GetMaxParticipantNum() const;
    virtual bool IsOpened() const;
    virtual Result GetApplicationData(void* buffer, unsigned int bufferSize) const;
    virtual unsigned int GetApplicationDataSize() const;
    virtual void Clear();
    virtual void Trace(unsigned long long value) const;
    virtual Result GetAttribute(unsigned char index, unsigned int* value) const;
    virtual unsigned short GetSessionType() const;
    virtual unsigned long long GetHostPrincipalId() const;
    virtual Result GetSessionStationInfo(LanStationInfo* stationInfo, unsigned int count);
    virtual unsigned int GetSessionStationInfoSize() const;
    virtual unsigned char GetApplicationCommunicationVersion() const;
    virtual unsigned char GetSystemCommunicationVersion() const;
    virtual void Copy(const LanSessionInfo& other);
    virtual void SetGameMode(unsigned int gameMode);
    virtual void SetSessionId(unsigned int sessionId);
    virtual void SetCurrentParticipantNum(unsigned short participantNum);
    virtual void SetMinParticipantNum(unsigned short participantNum);
    virtual void SetMaxParticipantNum(unsigned short participantNum);
    virtual void SetOpen(bool open);
    virtual void SetSessionType(LanSessionType sessionType);
    virtual void SetAttribute(unsigned char index, unsigned int value);
    virtual void SetApplicationData(const void* data, unsigned int size);
    virtual transport::StationConnectionInfo& GetStationConnectionInfo();
    virtual void SetStationConnectionInfo(const transport::StationConnectionInfo& info);
    virtual void AddSessionStationInfo(const LanStationInfo& stationInfo);
    virtual void ClearSessionStationInfo();
    virtual void SetSystemCommunicationVersion(unsigned char version);
    virtual void SetApplicationCommunicationVersion(unsigned char version);
    virtual const unsigned char* GetSessionParam() const;
    unsigned int GetSerializedSize() const;
    Result Serialize(unsigned char* buffer, unsigned int* size, unsigned int bufferSize) const;
    Result Deserialize(const unsigned char* buffer, unsigned int bufferSize);

    unsigned int m_gameMode;
    unsigned int m_sessionId;
    unsigned short m_currentParticipantNum;
    unsigned short m_minParticipantNum;
    unsigned short m_maxParticipantNum;
    bool m_open;
    unsigned int m_attribute[cAttributeCount];
    unsigned char m_systemCommunicationVersion;
    unsigned char m_applicationCommunicationVersion;
    unsigned short m_sessionType;
    unsigned char m_sessionParam[cSessionParamSize];
    unsigned char m_applicationData[cApplicationDataMaxSize];
    unsigned int m_applicationDataSize;
    transport::StationConnectionInfo m_stationConnectionInfo;
    LanStationInfo m_stationInfo[cMaxStationInfo];
    unsigned int m_stationInfoCount;
};

}
}
}

#endif
