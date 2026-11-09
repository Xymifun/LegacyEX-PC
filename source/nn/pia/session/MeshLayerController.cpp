#include "nn/pia/session/MeshLayerController.hpp"

#include <coreinit/memory.h>

#include "nn/pia/Transport.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn
{
namespace pia
{
namespace session
{

namespace
{
enum
{
    cResultInvalidArgument = 0x10c07,
    cHostMigrationModeRanking = 2,
    cIdentificationTokenSize = 0x20,
    cCryptoKeySize = 0x10
};

}

MeshLayerController::MeshLayerController()
    : m_playerInfoSize(0),
      m_registeredPlayerInfo(false),
      m_playersCountedAsParticipants(false),
      m_field84(0),
      m_field88(0),
      m_hostMigrationEnabled(false),
      m_bandwidthCheckMeasurementTime(0),
      m_bandwidthCheckIpPacketSize(0),
      m_skipBandwidthCheck(true),
      m_bandwidthCheckMeasurementCount(0),
      m_cryptoSetting(),
      m_payloadSize(0),
      m_recentPlayRecordEnabled(false),
      m_startedUp(false)
{
    ResetPlayerInfo();
    m_fielde0 = 0;
    OSBlockSet(&m_identificationToken, 0, cIdentificationTokenSize);
    m_cryptoSetting.m_mode = 0;
    OSBlockSet(m_cryptoSetting.m_key, 0, cCryptoKeySize);
}

Result MeshLayerController::Startup(const Session::StartupSetting& setting)
{
    if (!m_skipBandwidthCheck
        && !g_piaMesh->IsBandwidthCheckProtocolSetupParamValid(
            m_bandwidthCheckMeasurementTime, m_bandwidthCheckIpPacketSize, m_bandwidthCheckMeasurementCount))
        return cResultInvalidArgument;

    if (m_recentPlayRecordEnabled && g_piaMesh->IsRecentPlayRecordEnabled())
    {
        const Result result = g_piaMesh->CheckPlayerInfoSetting(m_registeredPlayerInfo ? m_playerInfo : 0,
                                                                m_playerInfoSize);
        if (result != 0)
            return result;
    }

    m_field84 = setting.m_field00;
    m_field88 = setting.m_field04;
    OSBlockSet(&m_identificationToken, 0, cIdentificationTokenSize);
    if (setting.m_identificationToken != 0)
        OSBlockMove(&m_identificationToken, setting.m_identificationToken, cIdentificationTokenSize, FALSE);
    transport::g_piaTransport->m_disconnectionEventListener = g_piaSession->GetDisconnectionEventListener();
    m_startedUp = true;
    return 0;
}

void MeshLayerController::Cleanup()
{
    transport::g_piaTransport->m_disconnectionEventListener = 0;
    m_playerInfoSize = 0;
    m_registeredPlayerInfo = false;
    m_playersCountedAsParticipants = false;
    m_field84 = 0;
    m_field88 = 0;
    m_hostMigrationEnabled = false;
    m_bandwidthCheckMeasurementTime = 0;
    m_bandwidthCheckIpPacketSize = 0;
    m_skipBandwidthCheck = true;
    m_bandwidthCheckMeasurementCount = 0;
    m_cryptoSetting.m_mode = 0;
    OSBlockSet(m_cryptoSetting.m_key, 0, cCryptoKeySize);
    m_payloadSize = 0;
    m_recentPlayRecordEnabled = false;
    m_startedUp = false;
    ResetPlayerInfo();
    m_fielde0 = 0;
    OSBlockSet(&m_identificationToken, 0, cIdentificationTokenSize);
}

Result MeshLayerController::MeshStartup(IMatchmakeSession* matchmakeSession, bool skipBandwidthCheck)
{
    g_piaMesh->RegisterEventListener(g_piaSession->GetMeshEventListenerPtr());
    g_piaMesh->RegisterJoiningMeshApprovalCallback(Session::SessionJoiningApprovalCallback);
    if (g_piaMesh->GetHostMigrationMode() == cHostMigrationModeRanking)
        g_piaMesh->RegisterCheckHostCandidateRankingCallback(Session::CheckHostCandidateRankingCallback);
    g_piaMesh->RegisterGetSessionIdCallback(Session::GetSessionIdCallback);

    Mesh::StartupSetting setting;
    setting.m_hostMigrationEnabled = m_hostMigrationEnabled;
    setting.m_playersCountedAsParticipants = m_playersCountedAsParticipants;
    setting.m_playerInfoSize = m_playerInfoSize;
    setting.m_playerInfo = m_registeredPlayerInfo ? m_playerInfo : 0;
    setting.m_field00 = m_field84;
    setting.m_field04 = m_field88;
    setting.m_field14 = m_registeredPlayerInfo ? &m_fielde0 : 0;
    setting.m_bandwidthCheckIpPacketSize = m_bandwidthCheckIpPacketSize;
    setting.m_bandwidthCheckMeasurementTime = m_bandwidthCheckMeasurementTime;
    setting.m_bandwidthCheckMeasurementCount = m_bandwidthCheckMeasurementCount;
    setting.m_cryptoSetting = &m_cryptoSetting;
    setting.m_recentPlayRecordEnabled = m_recentPlayRecordEnabled;
    if (!skipBandwidthCheck)
        skipBandwidthCheck = m_skipBandwidthCheck;
    setting.m_skipBandwidthCheck = skipBandwidthCheck;
    setting.m_identificationToken = &m_identificationToken;
    const Result result = g_piaMesh->Startup(setting);
    if (result != 0)
        MeshCleanup();
    return result;
}

void MeshLayerController::MeshCleanup()
{
    if (g_piaMesh != 0)
    {
        g_piaMesh->Cleanup();
        g_piaMesh->UnregisterEventListener();
        g_piaMesh->UnregisterJoiningMeshApprovalCallback();
        if (g_piaMesh->GetHostMigrationMode() == cHostMigrationModeRanking)
            g_piaMesh->UnregisterCheckHostCandidateRankingCallback();
        g_piaMesh->UnregisterGetSessionIdCallback();
    }
}

void MeshLayerController::UpdateCryptoSetting(const common::CryptoSetting& cryptoSetting)
{
    m_cryptoSetting.m_mode = cryptoSetting.m_mode;
    OSBlockMove(m_cryptoSetting.m_key, cryptoSetting.m_key, cCryptoKeySize, FALSE);
    transport::g_piaTransport->UpdateSetting(&m_cryptoSetting);
}

bool MeshLayerController::IsAllStationJoined(unsigned int)
{
    Session* session = g_piaSession;
    return session->GetMatchmakeSessionStationNum() == session->GetStationNum();
}

common::Job* MeshLayerController::GetJointSessionJobPtr()
{
    if (g_piaSession == 0)
        return 0;
    return g_piaSession->m_jointSessionJob;
}

void MeshLayerController::SetBufferSessionId(unsigned int sessionId)
{
    if (g_piaSession == 0)
        return;
    g_piaSession->m_sessionId[g_piaSession->m_currentSessionIndex == 0 ? 1 : 0] = sessionId;
}

void MeshLayerController::SetCurrentSessionId(unsigned int sessionId)
{
    if (g_piaSession == 0)
        return;
    g_piaSession->m_sessionId[g_piaSession->m_currentSessionIndex] = sessionId;
}

void MeshLayerController::Trace(unsigned long long) const
{
}

unsigned int MeshLayerController::GetMatchmakeSessionTotalStations() const
{
    return 0;
}

unsigned short MeshLayerController::GetMatchmakeSessionTotalParticipants() const
{
    return 0;
}

bool MeshLayerController::IsParticipated(unsigned int, unsigned long long) const
{
    return true;
}

bool MeshLayerController::IsJoinedUnknownSession(unsigned int, unsigned int)
{
    return false;
}

const transport::Station::PlayerInfo* MeshLayerController::GetPlayerInfo(unsigned char index) const
{
    return &m_playerInfo[index];
}

unsigned char MeshLayerController::GetPlayerInfoSize() const
{
    return m_playerInfoSize;
}

bool MeshLayerController::IsPlayersCountedAsParticipants() const
{
    return m_playersCountedAsParticipants;
}

bool MeshLayerController::IsRegisteredPlayerInfo() const
{
    return m_registeredPlayerInfo;
}

void MeshLayerController::SetLeaveFlag()
{
}

bool MeshLayerController::IsHostMigrationEnabled() const
{
    return m_hostMigrationEnabled;
}

bool MeshLayerController::IsStartedUp() const
{
    return m_startedUp;
}

}
}
}
