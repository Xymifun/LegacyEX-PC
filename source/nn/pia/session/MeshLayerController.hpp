#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_MESH_LAYER_CONTROLLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_MESH_LAYER_CONTROLLER_HPP

#include "nn/pia/common/CryptoSetting.hpp"
#include "nn/pia/common/Job.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/transport/Station.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class IMatchmakeSession;

class MeshLayerController
{
public:
    MeshLayerController();
    virtual ~MeshLayerController() {}
    virtual Result Startup(const Session::StartupSetting& setting);
    virtual void Cleanup();
    virtual Result MeshStartup(IMatchmakeSession* matchmakeSession, bool skipBandwidthCheck);
    virtual void MeshCleanup();
    virtual Result CheckConnectedStatus() = 0;
    virtual void Trace(unsigned long long value) const;
    virtual void SetBufferSessionId(unsigned int sessionId);
    virtual void SetCurrentSessionId(unsigned int sessionId);
    virtual unsigned int GetMatchmakeSessionTotalStations() const;
    virtual unsigned short GetMatchmakeSessionTotalParticipants() const;
    virtual bool IsParticipated(unsigned int sessionId, unsigned long long stationId) const;
    virtual common::Job* GetJointSessionJobPtr();
    virtual bool IsJoinedUnknownSession(unsigned int sessionId, unsigned int bufferSessionId);
    virtual const transport::Station::PlayerInfo* GetPlayerInfo(unsigned char index) const;
    virtual unsigned char GetPlayerInfoSize() const;
    virtual bool IsPlayersCountedAsParticipants() const;
    virtual bool IsRegisteredPlayerInfo() const;
    virtual void SetLeaveFlag();
    virtual bool IsAllStationJoined(unsigned int sessionId);
    virtual bool IsHostMigrationEnabled() const;
    virtual bool IsStartedUp() const;

    void UpdateCryptoSetting(const common::CryptoSetting& cryptoSetting);

protected:
    void ResetPlayerInfo()
    {
        m_playerInfo[0].m_name.Clear();
        m_playerInfo[0].m_language = 0xff;
        m_playerInfo[0].m_field70 = 0;
        m_playerInfo[0].m_field78 = 0;
    }

public:
    transport::Station::PlayerInfo m_playerInfo[1];
    unsigned char m_playerInfoSize;
    bool m_registeredPlayerInfo;
    bool m_playersCountedAsParticipants;
    unsigned int m_field84;
    unsigned int m_field88;
    bool m_hostMigrationEnabled;
    int m_bandwidthCheckMeasurementTime;
    unsigned int m_bandwidthCheckIpPacketSize;
    bool m_skipBandwidthCheck;
    int m_bandwidthCheckMeasurementCount;
    common::CryptoSetting m_cryptoSetting;
    unsigned int m_payloadSize;
    transport::Station::IdentificationToken m_identificationToken;
    bool m_recentPlayRecordEnabled;
    bool m_startedUp;
    unsigned long long m_fielde0;
};

}
}
}

#endif
