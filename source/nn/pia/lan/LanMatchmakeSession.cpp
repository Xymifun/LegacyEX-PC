#include "nn/pia/lan/LanMatchmakeSession.hpp"
#include "nn/pia/lan/LanFacade.hpp"
#include "nn/pia/lan/LanProtocol.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/lan/LanCreateSessionSetting.hpp"
#include "nn/pia/lan/LanJoinSessionSetting.hpp"
#include "nn/pia/lan/LanStationInfo.hpp"
#include "nn/pia/lan/LanUpdateSessionSetting.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/transport/IdentificationInfoTable.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "platform/wiiu/pead.hpp"

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
    cResultNotSupported = 0xc0b,
    cResultDisconnected = 0xc406,
    cResultSessionVoid = 0x6c4b,
    cResultSessionFull = 0x6c4c,
    cResultRemoteVersionOlder = 0x6470,
    cResultRemoteVersionNewer = 0x646f
};

enum
{
    cSystemCommunicationVersionMin = 5,
    cStationInfoRoleHost = 1,
    cStationInfoRoleClient = 2
};

enum
{
    cUpdateFlagApplicationData = 1 << session::UpdateSessionSetting::cModificationFlag_ApplicationData,
    cUpdateFlagMinParticipantNum = 1 << 2,
    cUpdateFlagMaxParticipantNum = 1 << 3,
    cUpdateFlagSessionType = 1 << 4,
    cUpdateFlagAttributeShift = 5
};

const unsigned long long cTraceFlagStationConnectionInfo = 0x0100000000000000ULL;
const unsigned long long cTraceFlagStation = 0x0200000000000000ULL;
}

LanMatchmakeSession::LanMatchmakeSession()
{
    m_sessionInfoList = session::g_piaSession->m_sessionInfoList;
    Clear();
}

LanMatchmakeSession::~LanMatchmakeSession()
{
    m_sessionInfoList = 0;
}

void LanMatchmakeSession::Clear()
{
    m_sessionInfo.Clear();
    m_sessionInfoBuffer.Clear();
    for (unsigned int i = 0; i < 2; ++i)
        m_searchCriteria[i].Reset();
    m_sessionInfoList->Clear();
    session::CommonMatchmakeSession::Cleanup();
    m_inquiryCallContext = 0;
    m_ignoredSessionId = 0;
    m_preferredSessionId = 0;
    m_matchmakeOwnerUniqueId = 0;
    m_sessionParamRegistered = false;
    m_matchmakeOwner = false;
    m_gatheringId = 0;
    m_callContext = 0;
}

void LanMatchmakeSession::Cleanup()
{
    if (m_callContext != 0)
        m_callContext->SignalCancel();
    if (m_inquiryCallContext != 0)
        m_inquiryCallContext->SignalCancel();
    if (m_ownedCallContext.m_state == common::CallContext::cStateInProgress)
        m_ownedCallContext.SignalCancel();
    Clear();
}

Result LanMatchmakeSession::RandomMatchmakeSession(common::CallContext* callContext)
{
    if (callContext == 0)
        return ResultInvalidArgument();
    if (callContext->m_state == common::CallContext::cStateInProgress)
        return ResultInvalidArgument();
    if (m_callContext != 0)
        return ResultInvalidState();
    m_sessionInfoList->Clear();
    m_gatheringId = 0;
    m_sessionParamRegistered = false;
    m_matchmakeOwnerUniqueId = 0;
    m_matchmakeOwner = false;
    transport::StationConnectionInfo stationConnectionInfo;
    m_sessionInfo.SetStationConnectionInfo(stationConnectionInfo);
    m_sessionInfo.SetCurrentParticipantNum(0);
    if (!LanFacade::GetInstance()->RequestBrowseMessage(m_searchCriteriaCount))
        return ResultInvalidState();
    m_callContext = callContext;
    callContext->InitiateCall();
    Result result = 0;
    return result;
}

void LanMatchmakeSession::SetSessionParam(const unsigned char* sessionParam, unsigned int sessionParamSize)
{
    m_sessionParamRegistered = true;
    OSBlockMove(m_sessionInfo.m_sessionParam, sessionParam, sessionParamSize, FALSE);
    m_publishedSessionInfo.Clear();
    m_publishedSessionInfo.Copy(m_sessionInfo);
}

bool LanMatchmakeSession::WaitRandomMatchmakeSession(unsigned int* gatheringId, bool* isNewSession)
{
    if (gatheringId == 0 || isNewSession == 0)
    {
        m_callContext->SignalFailure(ResultInvalidArgument());
        return true;
    }
    if (!LanFacade::GetInstance()->IsBrowseMessageCompleted())
        return false;
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        Result failure = cResultDisconnected;
        m_callContext->SignalFailure(failure);
        m_callContext = 0;
        return true;
    }
    LanSessionInfo* sessionInfo = 0;
    bool found = false;
    unsigned int participantNum = 1;
    session::MeshLayerController* meshLayerController = session::g_piaSession->m_meshLayerController;
    if (meshLayerController->IsRegisteredPlayerInfo() && meshLayerController->IsPlayersCountedAsParticipants())
        participantNum = meshLayerController->GetPlayerInfoSize();
    if (m_preferredSessionId != 0 && m_preferredSessionId != m_ignoredSessionId)
    {
        for (session::ISessionInfo** it = m_sessionInfoList->Begin(); it != m_sessionInfoList->End(); ++it)
        {
            sessionInfo = static_cast<LanSessionInfo*>(*it);
            if (sessionInfo->GetSessionId() == m_preferredSessionId)
            {
                bool isHit = false;
                for (int i = 0; i < m_searchCriteriaCount; ++i)
                    isHit |= m_searchCriteria[i].IsHit(sessionInfo);
                if (isHit && sessionInfo->IsOpened()
                    && sessionInfo->GetCurrentParticipantNum() + participantNum <= sessionInfo->GetMaxParticipantNum())
                {
                    found = true;
                    break;
                }
            }
        }
    }
    for (int i = 0; i < m_searchCriteriaCount && !found; ++i)
    {
        for (session::ISessionInfo** it = m_sessionInfoList->Begin(); it != m_sessionInfoList->End(); ++it)
        {
            sessionInfo = static_cast<LanSessionInfo*>(*it);
            if (m_searchCriteria[i].IsHit(sessionInfo) && sessionInfo->IsOpened()
                && sessionInfo->GetSessionId() != m_ignoredSessionId
                && sessionInfo->GetSystemCommunicationVersion() == m_sessionInfo.GetSystemCommunicationVersion()
                && sessionInfo->GetApplicationCommunicationVersion()
                       == m_sessionInfo.GetApplicationCommunicationVersion()
                && sessionInfo->GetCurrentParticipantNum() + participantNum <= sessionInfo->GetMaxParticipantNum())
            {
                found = true;
                break;
            }
        }
    }
    for (int i = 0; !found && i < m_searchCriteriaCount; ++i)
    {
        for (session::ISessionInfo** it = m_sessionInfoList->Begin(); it != m_sessionInfoList->End(); ++it)
        {
            sessionInfo = static_cast<LanSessionInfo*>(*it);
            if (m_searchCriteria[i].IsHit(sessionInfo) && sessionInfo->IsOpened()
                && sessionInfo->GetSessionId() != m_ignoredSessionId
                && sessionInfo->GetCurrentParticipantNum() + participantNum <= sessionInfo->GetMaxParticipantNum())
            {
                found = true;
                break;
            }
        }
    }
    if (!found)
    {
        m_matchmakeOwner = true;
        sessionInfo = &m_sessionInfo;
        m_gatheringId = (LanFacade::GetInstance()
                             ->GetLocalStationConnectionInfo()
                             .m_stationLocation.m_stationAddress.m_inetAddress.m_address
                         << 16)
                        | LanFacade::GetInstance()
                              ->GetLocalStationConnectionInfo()
                              .m_stationLocation.m_stationAddress.m_inetAddress.m_port;
        m_sessionInfo.SetStationConnectionInfo(LanFacade::GetInstance()->GetLocalStationConnectionInfo());
        m_sessionInfo.SetCurrentParticipantNum(participantNum);
        pead::Random random;
        for (unsigned int i = 0; i < LanSessionInfo::cSessionParamSize; ++i)
            m_sessionInfo.m_sessionParam[i] = random.getU32();
    }
    else
    {
        if (sessionInfo->GetSystemCommunicationVersion() != m_sessionInfo.GetSystemCommunicationVersion()
            || sessionInfo->GetApplicationCommunicationVersion() != m_sessionInfo.GetApplicationCommunicationVersion())
        {
            Result failure;
            if (sessionInfo->GetSystemCommunicationVersion() > m_sessionInfo.GetSystemCommunicationVersion()
                || sessionInfo->GetApplicationCommunicationVersion()
                       > m_sessionInfo.GetApplicationCommunicationVersion())
                failure = cResultRemoteVersionNewer;
            else
                failure = cResultRemoteVersionOlder;
            m_callContext->SignalFailure(failure);
            m_callContext = 0;
            m_sessionInfo.Clear();
            m_gatheringId = 0;
            m_publishedSessionInfo.Clear();
            return true;
        }
        m_matchmakeOwner = false;
        m_gatheringId = sessionInfo->GetSessionId();
        m_sessionInfo.Copy(*sessionInfo);
        m_sessionInfo.SetCurrentParticipantNum(m_sessionInfo.GetCurrentParticipantNum() + participantNum);
        SetSessionParam(sessionInfo->GetSessionParam(), LanSessionInfo::cSessionParamSize);
        LanFacade::GetInstance()->GetLanProtocol()->RegisterSessionKey();
    }
    m_matchmakeOwnerUniqueId = sessionInfo->GetHostPrincipalId();
    m_sessionInfo.SetSessionId(m_gatheringId);
    *gatheringId = m_gatheringId;
    *isNewSession = m_matchmakeOwner;
    m_publishedSessionInfo.Clear();
    m_publishedSessionInfo.Copy(m_sessionInfo);
    Result success = 0;
    m_callContext->SignalSuccess(success);
    m_callContext = 0;
    return true;
}

Result LanMatchmakeSession::BrowseMatchmakeSession(common::CallContext* callContext)
{
    if (callContext == 0)
        return ResultInvalidArgument();
    if (callContext->m_state == common::CallContext::cStateInProgress)
        return ResultInvalidArgument();
    if (m_callContext != 0)
        return ResultInvalidState();
    m_sessionInfoList->Clear();
    LanFacade* facade = LanFacade::GetInstance();
    if (!facade->RequestBrowseMessage(1))
        return ResultInvalidState();
    m_callContext = callContext;
    m_callContext->InitiateCall();
    return 0;
}

bool LanMatchmakeSession::WaitBrowseMatchmakeSession()
{
    if (!LanFacade::GetInstance()->IsBrowseMessageCompleted())
        return false;
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        m_callContext->SignalFailure(cResultDisconnected);
        m_callContext = 0;
        return true;
    }
    m_callContext->SignalSuccess(0);
    m_callContext = 0;
    return true;
}

Result LanMatchmakeSession::CreateMatchmakeSession(common::CallContext* callContext)
{
    if (!callContext)
        return ResultInvalidArgument();
    if (callContext->m_state == common::CallContext::cStateInProgress)
        return ResultInvalidArgument();
    if (m_callContext != 0)
        return ResultInvalidState();
    m_sessionInfoList->Clear();
    m_gatheringId = 0;
    m_matchmakeOwnerUniqueId = 0;
    m_matchmakeOwner = false;
    m_sessionParamRegistered = false;
    transport::StationConnectionInfo stationConnectionInfo;
    m_sessionInfo.SetStationConnectionInfo(stationConnectionInfo);
    m_sessionInfo.SetCurrentParticipantNum(0);
    m_callContext = callContext;
    callContext->InitiateCall();
    return 0;
}

bool LanMatchmakeSession::WaitCreateMatchmakeSession(unsigned int* gatheringId)
{
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        Result failure = cResultDisconnected;
        m_callContext->SignalFailure(failure);
        m_callContext = 0;
        return true;
    }
    m_gatheringId = (LanFacade::GetInstance()
                         ->GetLocalStationConnectionInfo()
                         .m_stationLocation.m_stationAddress.m_inetAddress.m_address
                     << 16)
                    | LanFacade::GetInstance()
                          ->GetLocalStationConnectionInfo()
                          .m_stationLocation.m_stationAddress.m_inetAddress.m_port;
    *gatheringId = m_gatheringId;
    m_sessionInfo.SetStationConnectionInfo(LanFacade::GetInstance()->GetLocalStationConnectionInfo());
    m_sessionInfo.SetSessionId(m_gatheringId);
    unsigned int participantNum = 1;
    session::MeshLayerController* meshLayerController = session::g_piaSession->m_meshLayerController;
    if (meshLayerController->IsRegisteredPlayerInfo() && meshLayerController->IsPlayersCountedAsParticipants())
        participantNum = meshLayerController->GetPlayerInfoSize();
    m_sessionInfo.SetCurrentParticipantNum(participantNum);
    pead::Random random;
    for (unsigned int i = 0; i < LanSessionInfo::cSessionParamSize; ++i)
        m_sessionInfo.m_sessionParam[i] = random.getU32();
    m_matchmakeOwner = true;
    m_matchmakeOwnerUniqueId = m_sessionInfo.GetHostPrincipalId();
    Result success = 0;
    m_callContext->SignalSuccess(success);
    m_callContext = 0;
    m_publishedSessionInfo.Clear();
    m_publishedSessionInfo.Copy(m_sessionInfo);
    return true;
}

Result LanMatchmakeSession::JoinMatchmakeSession(common::CallContext* callContext, unsigned int gatheringId)
{
    if (callContext == 0)
        return ResultInvalidArgument();
    if (callContext->m_state == common::CallContext::cStateInProgress)
        return ResultInvalidArgument();
    if (m_callContext != 0)
        return ResultInvalidState();
    m_matchmakeOwner = false;
    m_gatheringId = gatheringId;
    m_matchmakeOwnerUniqueId = 0;
    LanFacade* facade = LanFacade::GetInstance();
    LanProtocol* protocol = facade->GetLanProtocol();
    if (!protocol->RequestGetSessionMessage())
        return ResultInvalidState();
    m_callContext = callContext;
    m_callContext->InitiateCall();
    return 0;
}

bool LanMatchmakeSession::WaitJoinMatchmakeSession()
{
    if (!LanFacade::GetInstance()->GetLanProtocol()->IsGetSessionMessageCompleted())
        return false;
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        Result failure = cResultDisconnected;
        m_callContext->SignalFailure(failure);
        m_callContext = 0;
        return true;
    }
    Result sessionResult = LanFacade::GetInstance()->GetLanProtocol()->GetSessionMessageResult();
    if (sessionResult != 0)
    {
        Result failure = cResultSessionVoid;
        m_callContext->SignalFailure(failure);
        m_callContext = 0;
        return true;
    }
    unsigned int participantNum = 1;
    int localVersion = LanFacade::GetInstance()->GetApplicationCommunicationVersion();
    int applicationVersion = m_sessionInfo.GetApplicationCommunicationVersion();
    int systemVersion = m_sessionInfo.GetSystemCommunicationVersion();
    Result result = cResultSessionVoid;
    if (systemVersion < cSystemCommunicationVersionMin)
        result = cResultRemoteVersionOlder;
    else if (systemVersion > cSystemCommunicationVersionMin)
        result = cResultRemoteVersionNewer;
    else if (applicationVersion < localVersion)
        result = cResultRemoteVersionOlder;
    else if (applicationVersion > localVersion)
        result = cResultRemoteVersionNewer;
    else if (m_sessionInfo.GetSessionId() == m_gatheringId && m_sessionInfo.IsOpened())
    {
        session::MeshLayerController* meshLayerController = session::g_piaSession->m_meshLayerController;
        if (meshLayerController->IsRegisteredPlayerInfo() && meshLayerController->IsPlayersCountedAsParticipants())
            participantNum = meshLayerController->GetPlayerInfoSize();
        unsigned int currentParticipantNum = m_sessionInfo.GetCurrentParticipantNum();
        unsigned int maxParticipantNum = m_sessionInfo.GetMaxParticipantNum();
        if (currentParticipantNum + participantNum <= maxParticipantNum)
        {
            m_matchmakeOwnerUniqueId = m_sessionInfo.GetHostPrincipalId();
            m_sessionInfo.SetCurrentParticipantNum(m_sessionInfo.GetCurrentParticipantNum() + participantNum);
            m_sessionInfo.GetStationConnectionInfo().Trace(cTraceFlagStationConnectionInfo);
            Result success = 0;
            m_callContext->SignalSuccess(success);
            m_callContext = 0;
            m_publishedSessionInfo.Clear();
            m_publishedSessionInfo.Copy(m_sessionInfo);
            return true;
        }
        result = cResultSessionFull;
    }
    m_sessionInfo.Clear();
    m_gatheringId = 0;
    m_callContext->SignalFailure(result);
    m_callContext = 0;
    m_publishedSessionInfo.Clear();
    m_publishedSessionInfo.Copy(m_sessionInfo);
    return true;
}

Result LanMatchmakeSession::LeaveMatchmakeSession(common::CallContext* callContext, unsigned int gatheringId)
{
    if (callContext == 0)
        return ResultInvalidArgument();
    if (callContext->m_state == common::CallContext::cStateInProgress)
        return ResultInvalidArgument();
    if (m_callContext != 0)
        return ResultInvalidState();
    if (m_gatheringId != gatheringId)
        return ResultInvalidArgument();
    m_callContext = callContext;
    m_callContext->InitiateCall();
    return 0;
}

bool LanMatchmakeSession::WaitLeaveMatchmakeSession()
{
    m_matchmakeOwner = false;
    m_matchmakeOwnerUniqueId = 0;
    m_gatheringId = 0;
    transport::StationConnectionInfo stationConnectionInfo;
    m_sessionInfo.SetStationConnectionInfo(stationConnectionInfo);
    session::CommonMatchmakeSession::Cleanup();
    m_publishedSessionInfo.Clear();
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        Result failure = cResultDisconnected;
        m_callContext->SignalFailure(failure);
        m_callContext = 0;
        return true;
    }
    Result success = 0;
    m_callContext->SignalSuccess(success);
    m_callContext = 0;
    return true;
}

Result LanMatchmakeSession::UnregisterMatchmakeSession(common::CallContext* callContext, unsigned int gatheringId)
{
    if (callContext == 0)
        return ResultInvalidArgument();
    if (callContext->m_state == common::CallContext::cStateInProgress)
        return ResultInvalidArgument();
    if (m_callContext != 0)
        return ResultInvalidState();
    if (m_gatheringId != gatheringId)
        return ResultInvalidArgument();
    m_callContext = callContext;
    m_sessionInfo.Clear();
    m_callContext->InitiateCall();
    return 0;
}

bool LanMatchmakeSession::WaitUnregisterMatchmakeSession()
{
    m_matchmakeOwner = false;
    m_matchmakeOwnerUniqueId = 0;
    m_gatheringId = 0;
    transport::StationConnectionInfo stationConnectionInfo;
    m_sessionInfo.SetStationConnectionInfo(stationConnectionInfo);
    session::CommonMatchmakeSession::Cleanup();
    m_publishedSessionInfo.Clear();
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        Result failure = cResultDisconnected;
        m_callContext->SignalFailure(failure);
        m_callContext = 0;
        return true;
    }
    Result success = 0;
    m_callContext->SignalSuccess(success);
    m_callContext = 0;
    return true;
}

Result LanMatchmakeSession::OpenParticipate(common::CallContext* callContext, unsigned int gatheringId)
{
    if (callContext == 0)
        return ResultInvalidArgument();
    if (callContext->m_state == common::CallContext::cStateInProgress)
        return ResultInvalidArgument();
    if (m_callContext != 0)
        return ResultInvalidState();
    if (m_gatheringId != gatheringId)
        return ResultInvalidArgument();
    LanFacade* facade = LanFacade::GetInstance();
    bool requested = facade->GetLanProtocol()->RequestUpdateSessionMessage();
    if (!requested)
        return ResultInvalidState();
    m_sessionInfo.SetOpen(true);
    m_callContext = callContext;
    m_callContext->InitiateCall();
    Result result = 0;
    return result;
}

bool LanMatchmakeSession::WaitOpenParticipate()
{
    if (!LanFacade::GetInstance()->GetLanProtocol()->IsUpdateSessionMessageCompleted())
        return false;
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        Result failure = cResultDisconnected;
        m_callContext->SignalFailure(failure);
        m_callContext = 0;
        return true;
    }
    Result success = 0;
    m_callContext->SignalSuccess(success);
    m_callContext = 0;
    return true;
}

Result LanMatchmakeSession::CloseParticipate(common::CallContext* callContext, unsigned int gatheringId)
{
    if (callContext == 0)
        return ResultInvalidArgument();
    if (callContext->m_state == common::CallContext::cStateInProgress)
        return ResultInvalidArgument();
    if (m_callContext != 0)
        return ResultInvalidState();
    if (m_gatheringId != gatheringId)
        return ResultInvalidArgument();
    LanFacade* facade = LanFacade::GetInstance();
    bool requested = facade->GetLanProtocol()->RequestUpdateSessionMessage();
    if (!requested)
        return ResultInvalidState();
    m_sessionInfo.SetOpen(false);
    m_callContext = callContext;
    m_callContext->InitiateCall();
    Result result = 0;
    return result;
}

bool LanMatchmakeSession::WaitCloseParticipate()
{
    if (!LanFacade::GetInstance()->GetLanProtocol()->IsUpdateSessionMessageCompleted())
        return false;
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        Result failure = cResultDisconnected;
        m_callContext->SignalFailure(failure);
        m_callContext = 0;
        return true;
    }
    Result success = 0;
    m_callContext->SignalSuccess(success);
    m_callContext = 0;
    return true;
}

Result LanMatchmakeSession::GetStationConnectionInfo(common::CallContext* callContext, unsigned int gatheringId)
{
    if (callContext == 0)
        return ResultInvalidArgument();
    if (callContext->m_state == common::CallContext::cStateInProgress)
        return ResultInvalidArgument();
    if (m_callContext != 0)
        return ResultInvalidState();
    if (m_gatheringId != gatheringId)
        return ResultInvalidArgument();
    if (!LanFacade::GetInstance()->GetLanProtocol()->RequestGetHostMessage())
        return ResultInvalidState();
    m_callContext = callContext;
    m_callContext->InitiateCall();
    transport::StationConnectionInfo stationConnectionInfo;
    m_sessionInfo.SetStationConnectionInfo(stationConnectionInfo);
    return 0;
}

bool LanMatchmakeSession::WaitGetStationConnectionInfo(transport::StationConnectionInfo* stationConnectionInfo)
{
    if (!LanFacade::GetInstance()->GetLanProtocol()->IsGetHostMessageCompleted())
        return false;
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        Result failure = cResultDisconnected;
        m_callContext->SignalFailure(failure);
        m_callContext = 0;
        return true;
    }
    transport::StationConnectionInfo emptyStationConnectionInfo;
    Result result = 0;
    if (m_sessionInfo.GetStationConnectionInfo() == emptyStationConnectionInfo)
        result = cResultSessionVoid;
    else
        *stationConnectionInfo = m_sessionInfo.GetStationConnectionInfo();
    if (result == 0)
        m_callContext->SignalSuccess(result);
    else
        m_callContext->SignalFailure(result);
    m_callContext = 0;
    return true;
}

Result LanMatchmakeSession::RequestSessionInfo(common::CallContext* callContext, unsigned int gatheringId)
{
    if (callContext == 0)
        return ResultInvalidArgument();
    if (callContext->m_state == common::CallContext::cStateInProgress)
        return ResultInvalidArgument();
    if (m_callContext != 0)
        return ResultInvalidState();
    if (m_gatheringId != gatheringId)
        return ResultInvalidArgument();
    if (!m_matchmakeOwner)
    {
        if (!LanFacade::GetInstance()->GetLanProtocol()->RequestGetSessionMessage())
            return ResultInvalidState();
    }
    else
    {
        UpdateStationInfoList();
    }
    m_callContext = callContext;
    m_callContext->InitiateCall();
    Result result = 0;
    return result;
}

bool LanMatchmakeSession::WaitRequestSessionInfo()
{
    if (!m_matchmakeOwner)
    {
        if (!LanFacade::GetInstance()->GetLanProtocol()->IsGetSessionMessageCompleted())
            return false;
        if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
        {
            Result failure = cResultDisconnected;
            m_callContext->SignalFailure(failure);
            m_callContext = 0;
            return true;
        }
        Result sessionResult = LanFacade::GetInstance()->GetLanProtocol()->GetSessionMessageResult();
        if (sessionResult != 0)
        {
            m_callContext->SignalFailure(LanFacade::GetInstance()->GetLanProtocol()->GetSessionMessageResult());
            m_callContext = 0;
            return true;
        }
    }
    else if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        Result failure = cResultDisconnected;
        m_callContext->SignalFailure(failure);
        m_callContext = 0;
        return true;
    }
    Result success = 0;
    m_callContext->SignalSuccess(success);
    m_callContext = 0;
    m_publishedSessionInfo.Copy(m_sessionInfo);
    return true;
}

Result LanMatchmakeSession::UpdateMatchmakeSessionHost(unsigned int stationId)
{
    return cResultNotSupported;
}

void LanMatchmakeSession::UpdateStationInfoList()
{
    if (!m_matchmakeOwner)
        return;
    m_sessionInfo.ClearSessionStationInfo();
    session::Session* piaSession = session::g_piaSession;
    common::ListNode* sentinel = piaSession->m_validStationIdList.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        transport::Station* station = transport::StationManager::s_pInstance->GetStation(
            *session::Session::StationIdList::ValueOf(node));
        if (station == 0)
            continue;
        LanStationInfo stationInfo;
        station->Trace(cTraceFlagStation);
        if (station->m_field10
            == static_cast<unsigned int>(transport::StationManager::s_pInstance->m_invalidStationIndex))
            stationInfo.SetRole(static_cast<session::SessionStationInfo::StationInfoRole>(cStationInfoRoleHost));
        else
            stationInfo.SetRole(static_cast<session::SessionStationInfo::StationInfoRole>(cStationInfoRoleClient));
        transport::IdentificationInfo identificationInfo;
        Result result = transport::IdentificationInfoTable::GetInstance()->GetIdentificationInfo(station,
                                                                                                 &identificationInfo);
        if (result == 0)
        {
            stationInfo.SetUserName(identificationInfo.m_fieldC8[0]);
            stationInfo.SetStationId(station->m_stationId);
        }
        else
        {
            stationInfo.SetStationId(0);
        }
        m_sessionInfo.AddSessionStationInfo(stationInfo);
    }
}

const session::ISessionInfo* LanMatchmakeSession::GetSessionInfo() const
{
    return &m_publishedSessionInfo;
}

session::ISessionInfo* LanMatchmakeSession::GetSessionInfo()
{
    return &m_publishedSessionInfo;
}

Result LanMatchmakeSession::UpdateSessionSetting(common::CallContext* callContext, unsigned int gatheringId)
{
    if (callContext == 0)
        return ResultInvalidArgument();
    if (callContext->m_state == common::CallContext::cStateInProgress)
        return ResultInvalidArgument();
    if (m_callContext != 0)
        return ResultInvalidState();
    if (!LanFacade::GetInstance()->GetLanProtocol()->RequestUpdateSessionMessage())
        return ResultInvalidState();
    m_callContext = callContext;
    m_callContext->InitiateCall();
    return 0;
}

bool LanMatchmakeSession::WaitUpdateSessionSetting()
{
    if (!LanFacade::GetInstance()->GetLanProtocol()->IsUpdateSessionMessageCompleted())
        return false;
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        Result failure = cResultDisconnected;
        m_callContext->SignalFailure(failure);
        m_callContext = 0;
        return true;
    }
    Result success = 0;
    m_callContext->SignalSuccess(success);
    m_callContext = 0;
    return true;
}

Result LanMatchmakeSession::InquiryOwnerPrincipalId(common::CallContext* callContext, unsigned int gatheringId)
{
    if (callContext == 0)
        return ResultInvalidArgument();
    if (callContext->m_state == common::CallContext::cStateInProgress)
        return ResultInvalidArgument();
    if (m_inquiryCallContext != 0)
        return ResultInvalidState();
    if (m_gatheringId != gatheringId)
        return ResultInvalidArgument();
    m_matchmakeOwnerUniqueId = static_cast<unsigned long long>(-1);
    if (!LanFacade::GetInstance()->GetLanProtocol()->RequestGetHostMessage())
        return ResultInvalidState();
    m_inquiryCallContext = callContext;
    m_inquiryCallContext->InitiateCall();
    Result result = 0;
    return result;
}

bool LanMatchmakeSession::WaitInquiryOwnerPrincipalId()
{
    if (!LanFacade::GetInstance()->GetLanProtocol()->IsGetHostMessageCompleted())
        return false;
    if (m_inquiryCallContext != 0)
    {
        m_inquiryCallContext->SignalSuccess(0);
        m_inquiryCallContext = 0;
    }
    return true;
}

Result LanMatchmakeSession::CancelInquiryOwnerPrincipalId()
{
    if (m_inquiryCallContext != 0)
    {
        m_inquiryCallContext->SignalCancel();
        m_inquiryCallContext = 0;
    }
    return 0;
}

unsigned int LanMatchmakeSession::GetGatheringId() const
{
    return m_gatheringId;
}

LanSessionInfo* LanMatchmakeSession::GetSessionInfoPtr()
{
    return &m_sessionInfo;
}

LanSessionInfo* LanMatchmakeSession::GetSessionInfoBufferPtr()
{
    return &m_sessionInfoBuffer;
}

session::ISessionInfoList* LanMatchmakeSession::GetSessionInfoListPtr()
{
    return m_sessionInfoList;
}

session::ISessionInfoList* LanMatchmakeSession::GetBrowseMatchmakeSessionResult()
{
    return m_sessionInfoList;
}

LanSessionSearchCriteria* LanMatchmakeSession::GetSessionSearchCriteriaPtr(unsigned char index)
{
    if (index >= 2)
        return 0;
    return &m_searchCriteria[index];
}

void LanMatchmakeSession::ReflectSettingToMatchmakeSession(const session::CreateSessionSetting* setting)
{
    const LanCreateSessionSetting* createSetting = static_cast<const LanCreateSessionSetting*>(setting);
    m_sessionInfo.Clear();
    if (createSetting->IsSetGameMode())
        m_sessionInfo.SetGameMode(createSetting->GetGameMode());
    m_sessionInfo.SetSessionId(LanFacade::GetInstance()
                                   ->GetLocalStationConnectionInfo()
                                   .m_stationLocation.m_stationAddress.m_inetAddress.m_port
                               | (LanFacade::GetInstance()
                                      ->GetLocalStationConnectionInfo()
                                      .m_stationLocation.m_stationAddress.m_inetAddress.m_address
                                  << 16));
    for (unsigned int i = 0; i < LanSessionInfo::cAttributeCount; ++i)
    {
        if (createSetting->IsSetAttribute(i))
            m_sessionInfo.SetAttribute(i, createSetting->GetAttribute(i));
    }
    if (session::g_piaSession != 0)
    {
        unsigned int participantNum = session::g_piaSession->GetStationNum();
        session::MeshLayerController* meshLayerController = session::g_piaSession->m_meshLayerController;
        if (meshLayerController->IsRegisteredPlayerInfo() && meshLayerController->IsPlayersCountedAsParticipants())
            participantNum = meshLayerController->GetPlayerInfoSize();
        m_sessionInfo.SetCurrentParticipantNum(participantNum);
    }
    else
    {
        m_sessionInfo.SetCurrentParticipantNum(0);
    }
    if (createSetting->IsSetMinParticipantNum())
        m_sessionInfo.SetMinParticipantNum(createSetting->GetMinParticipantNum());
    if (createSetting->IsSetMaxParticipantNum())
        m_sessionInfo.SetMaxParticipantNum(createSetting->GetMaxParticipantNum());
    if (createSetting->IsSetSessionType())
        m_sessionInfo.SetSessionType(createSetting->GetSessionType());
    if (createSetting->IsSetApplicationData())
    {
        Result result;
        unsigned char applicationData[LanSessionInfo::cApplicationDataMaxSize];
        unsigned int applicationDataSize = createSetting->GetApplicationDataSize();
        if (applicationDataSize != 0)
            result = createSetting->GetApplicationData(applicationData, LanSessionInfo::cApplicationDataMaxSize);
        m_sessionInfo.SetApplicationData(applicationData, applicationDataSize);
    }
    if (createSetting->IsSetOpenSession())
        m_sessionInfo.SetOpen(createSetting->IsOpenSession());
    m_sessionInfo.SetSystemCommunicationVersion(cSystemCommunicationVersionMin);
    m_sessionInfo.SetApplicationCommunicationVersion(LanFacade::GetInstance()->GetApplicationCommunicationVersion());
}

void LanMatchmakeSession::ReflectSettingToMatchmakeSession(const session::JoinSessionSetting* setting)
{
    const LanSessionInfo* sessionInfo = static_cast<const LanSessionInfo*>(setting->GetSessionInfoPtr());
    m_sessionInfo.Copy(*sessionInfo);
    m_sessionParamRegistered = true;
    m_publishedSessionInfo.Clear();
    m_publishedSessionInfo.Copy(m_sessionInfo);
    LanFacade::GetInstance()->GetLanProtocol()->RegisterSessionKey();
}

void LanMatchmakeSession::ReflectSettingToMatchmakeSession(const session::UpdateSessionSetting* setting, bool open)
{
    const LanUpdateSessionSetting* updateSetting = static_cast<const LanUpdateSessionSetting*>(setting);
    unsigned int modificationFlag = updateSetting->GetModificationFlag();
    if ((modificationFlag & cUpdateFlagSessionType) != 0)
        m_sessionInfo.SetSessionType(updateSetting->GetSessionType());
    if ((modificationFlag & cUpdateFlagApplicationData) != 0)
    {
        unsigned int applicationDataSize = updateSetting->GetApplicationDataSize();
        if (applicationDataSize != 0)
        {
            Result result;
            unsigned char applicationData[LanSessionInfo::cApplicationDataMaxSize];
            result = updateSetting->GetApplicationData(applicationData, LanSessionInfo::cApplicationDataMaxSize);
            m_sessionInfo.SetApplicationData(applicationData, applicationDataSize);
        }
    }
    if (open)
        m_sessionInfo.SetOpen(true);
    if ((modificationFlag & cUpdateFlagMinParticipantNum) != 0)
        m_sessionInfo.SetMinParticipantNum(updateSetting->GetMinParticipantNum());
    if ((modificationFlag & cUpdateFlagMaxParticipantNum) != 0)
        m_sessionInfo.SetMaxParticipantNum(updateSetting->GetMaxParticipantNum());
    for (unsigned int i = 0; i < LanSessionInfo::cAttributeCount; ++i)
    {
        if ((modificationFlag & (1 << (cUpdateFlagAttributeShift + i))) != 0)
            m_sessionInfo.SetAttribute(i, updateSetting->GetAttribute(i));
    }
}

unsigned int LanMatchmakeSession::RegisterSearchCriteria(const session::SessionSearchCriteria* criteria,
                                                         unsigned char count)
{
    m_searchCriteriaCount = count > 2 ? 2 : count;
    const LanSessionSearchCriteria* source = static_cast<const LanSessionSearchCriteria*>(criteria);
    for (unsigned char i = 0; i < m_searchCriteriaCount; ++i)
        m_searchCriteria[i] = *source++;
    return 1;
}

void LanMatchmakeSession::UnregisterSearchCriteria()
{
    for (unsigned int i = 0; i < 2; ++i)
        m_searchCriteria[i].Reset();
    m_searchCriteriaCount = 0;
}

void LanMatchmakeSession::SetIgnoredSessionId(unsigned int sessionId)
{
    m_ignoredSessionId = sessionId;
}

void LanMatchmakeSession::SetPreferredSessionId(unsigned int sessionId)
{
    m_preferredSessionId = sessionId;
}

void LanMatchmakeSession::CopySessionInfo()
{
    m_publishedSessionInfo.Clear();
    m_publishedSessionInfo.Copy(m_sessionInfo);
}

bool LanMatchmakeSession::IsSessionParamRegistered() const
{
    return m_sessionParamRegistered;
}

void LanMatchmakeSession::Trace(unsigned long long value) const
{
}

}
}
}
