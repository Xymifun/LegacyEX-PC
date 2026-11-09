#include "nn/pia/lan/LanMatchMeshLayerController.hpp"

#include <cstring>

#include <coreinit/memory.h>

#include "nn/pia/lan/LanFacade.hpp"
#include "nn/pia/lan/LanSessionStartupSetting.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/transport/Station.hpp"
#include "platform/wiiu/pead.hpp"
#include "nn/pia/transport/StationManager.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

namespace
{
class ResultDisconnectedNetwork : public Result
{
public:
    ResultDisconnectedNetwork() : Result(session::Session::cResultStatusDisconnectedNetwork) {}
};

class ResultDisconnectedSession : public Result
{
public:
    ResultDisconnectedSession() : Result(session::Session::cResultStatusDisconnectedSession) {}
};

enum
{
    cPayloadSize = 0x554,
    cCryptoKeySize = 0x10
};
}

LanMatchMeshLayerController::LanMatchMeshLayerController() : session::MeshLayerController()
{
    m_matchmakeSession = 0;
}

LanMatchMeshLayerController::~LanMatchMeshLayerController()
{
}

Result LanMatchMeshLayerController::Startup(const session::Session::StartupSetting& baseSetting)
{
    const LanSessionStartupSetting& setting = static_cast<const LanSessionStartupSetting&>(baseSetting);
    const Result playerInfoResult = session::g_piaMesh->CheckPlayerInfo(setting.m_playerInfo[0]);
    const bool playerInfoValid = playerInfoResult == 0;
    m_payloadSize = cPayloadSize;
    m_registeredPlayerInfo = false;
    m_playerInfoSize = 0;
    ResetPlayerInfo();
    if (playerInfoValid)
    {
        transport::Station::PlayerInfo& playerInfo = m_playerInfo[m_playerInfoSize];
        const transport::Station::PlayerInfo& sourceInfo = setting.m_playerInfo[m_playerInfoSize];
        m_registeredPlayerInfo = true;
        playerInfo.m_name.Copy(sourceInfo.m_name);
        playerInfo.m_language = sourceInfo.m_language;
        playerInfo.m_field70 = sourceInfo.m_field70;
        playerInfo.m_field78 = sourceInfo.m_field78;
        m_playerInfo[m_playerInfoSize].m_name.Trace(0x4000000ULL);
        ++m_playerInfoSize;
    }
    if (m_playerInfoSize == 0)
        m_playerInfoSize = 1;
    m_playersCountedAsParticipants = setting.m_playersCountedAsParticipants;
    m_cryptoSetting.m_mode = setting.m_cryptoSetting.m_mode;
    pead::Random random;
    for (unsigned int index = 0; index < cCryptoKeySize; ++index)
        m_cryptoSetting.m_key[index] = static_cast<unsigned char>(random.getU32());
    m_bandwidthCheckMeasurementCount = 0;
    m_recentPlayRecordEnabled = false;
    m_hostMigrationEnabled = true;
    m_bandwidthCheckIpPacketSize = 0;
    m_skipBandwidthCheck = true;
    m_bandwidthCheckMeasurementTime = 0;

    LanFacade* facade = LanFacade::GetInstance();
    if (facade == 0)
        return ResultInvalidState();
    if (setting.m_cryptoSetting.m_mode == 0)
        return ResultInvalidArgument();

    unsigned char zeroKey[cCryptoKeySize];
    OSBlockSet(zeroKey, 0, sizeof(zeroKey));
    if (::memcmp(setting.m_cryptoSetting.m_key, zeroKey, sizeof(zeroKey)) == 0)
        return ResultInvalidArgument();

    Result result = facade->Startup(setting.m_cryptoSetting);
    if (result != 0)
        return result;
    result = facade->StartLanSession();
    if (result != 0)
        return result;
    m_connectionStatus = session::Session::cConnectionStatusConnected;
    return session::MeshLayerController::Startup(baseSetting);
}

void LanMatchMeshLayerController::MeshCleanup()
{
    session::MeshLayerController::MeshCleanup();
    m_matchmakeSession = 0;
}

void LanMatchMeshLayerController::Cleanup()
{
    if (LanFacade::GetInstance() != 0)
    {
        LanFacade::GetInstance()->StopLanSession();
        LanFacade::GetInstance()->Cleanup();
    }
    LanMatchMeshLayerController::MeshCleanup();
    session::MeshLayerController::Cleanup();
}

Result LanMatchMeshLayerController::MeshStartup(session::IMatchmakeSession* matchmakeSession, bool skipBandwidthCheck)
{
    m_matchmakeSession = matchmakeSession;
    return session::MeshLayerController::MeshStartup(matchmakeSession, false);
}

Result LanMatchMeshLayerController::CheckConnectedStatus()
{
    const session::Session::ConnectionStatus status = m_connectionStatus;
    if (status == session::Session::cConnectionStatusDisconnectedNetwork)
        return ResultDisconnectedNetwork();
    if (status == session::Session::cConnectionStatusDisconnectedSession)
        return ResultDisconnectedSession();
    return 0;
}

void LanMatchMeshLayerController::Trace(unsigned long long) const
{
}

unsigned int LanMatchMeshLayerController::GetMatchmakeSessionTotalStations() const
{
    return session::g_piaSession->GetStationNum();
}

unsigned short LanMatchMeshLayerController::GetMatchmakeSessionTotalParticipants() const
{
    unsigned short total = 0;
    common::ListNode* sentinel = session::g_piaSession->m_validStationIdList.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        transport::Station* station = transport::StationManager::s_pInstance->GetStation(
            *session::Session::StationIdList::ValueOf(node));
        unsigned char count;
        if (station != 0 && station->GetParticipantCount(&count) == 0)
            total += count;
    }
    return total;
}

bool LanMatchMeshLayerController::IsParticipated(unsigned int, unsigned long long principalId) const
{
    const transport::StationManager* manager = transport::StationManager::s_pInstance;
    transport::Station* const* begin = manager->m_stations.data;
    for (transport::Station* const* it = begin; it != begin + manager->m_stations.size; ++it)
    {
        unsigned long long stationPrincipalId;
        if ((*it)->GetPrincipalId(&stationPrincipalId) == 0 && stationPrincipalId == principalId)
            return true;
    }
    return false;
}

void LanMatchMeshLayerController::SetConnectionStatusDisconnected()
{
    m_connectionStatus = session::Session::cConnectionStatusDisconnectedNetwork;
}

}
}
}
