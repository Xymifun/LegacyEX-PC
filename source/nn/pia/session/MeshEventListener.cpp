#include "nn/pia/session/MeshEventListener.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Job.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/JoinSessionJob.hpp"
#include "nn/pia/session/JointSessionJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/ProcessDestroyMeshJob.hpp"
#include "nn/pia/session/ProcessHostMigrationJob.hpp"
#include "nn/pia/session/RandomMatchmakeJob.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/session/StationIdStatusTable.hpp"
#include "nn/pia/transport/DisconnectStationJob.hpp"
#include "nn/pia/transport/IdentificationInfoTable.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationManager.hpp"

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
    cMeshEventKindJoin = 0,
    cMeshEventKindLeave = 1,
    cMeshEventKind2 = 2,
    cMeshEventKind3 = 3,
    cMeshEventKind4 = 4,
    cMeshEventKind19 = 0x13,
    cMeshEventKind20 = 0x14,
    cMeshEventKind21 = 0x15,
    cMeshEventKind22 = 0x16,
    cMeshEventKind23 = 0x17,
    cMeshEventKind24 = 0x18,
    cMeshEventKind25 = 0x19,
    cMeshEventKindCheckStatus = 0x12,
    cMatchmakeStatusUnknown2 = 2,
    cMatchmakeStatusUnknown3 = 3,
    cMatchmakeStatusUnknown4 = 4,
    cConnectionStatusDisconnectedSession = 2,
    cDisconnectReasonJoinFailed = 5,
    cResultOwnerChanged = 0x6c59,
    cTraceMaskStationTable = 0x4000000,
    cStatusMatchedCurrent = 1,
    cStatusUnknown2 = 2,
    cStatusMatchedBuffer = 3,
    cDisconnectReasonJointSession = 7,
    cDisconnectReasonOwnerChanged = 9,
    cLeaveSessionDisconnectReason = 2,
    cStationLeaveStateRequested = 5,
    cStationLeaveStateLeaving = 6
};
}

MeshEventListener::MeshEventListener()
{
}

MeshEventListener::~MeshEventListener()
{
}

Result MeshEventListener::RemoveStationId(unsigned long long stationId)
{
    Session* session = g_piaSession;
    Result result = 0;
    if (session->m_stationIdStatusTable->CheckEventNotifiedStatus(stationId))
    {
        if (session->m_sessionEventCallback != 0)
            session->m_sessionEventCallback(Session::cEventTypeLeave, stationId, session->m_sessionEventCallbackParam);
        session->invalidateStationId(stationId);
    }
    result = transport::g_piaTransport->m_stationIdTable->RemoveByStationId(stationId);
    result = session->m_stationIdStatusTable->RemoveElement(stationId);
    return result;
}

void MeshEventListener::RemoveStationIndex(StationIndex stationIndex)
{
    g_piaMesh->UnfixDisconnectedId(stationIndex);
    transport::Station* station = transport::StationManager::s_pInstance->GetStation(stationIndex);
    if (station != 0)
    {
        station->m_disconnectStationJob->ProcessDisconnection(station);
        transport::g_stationConnectionInfoTable->EraseFromTable(station);
        transport::IdentificationInfoTable::GetInstance()->EraseFromTable(station);
        station->CleanupJobs();
        station->Cleanup();
        transport::StationManager::s_pInstance->DestroyStation(station);
        transport::g_piaTransport->OutputStreamUpdateEvent();
    }
}

void MeshEventListener::NoticeEvent(const MeshEvent& event)
{
    Session* session = g_piaSession;
    transport::StationIdTable* stationIdTable = transport::g_piaTransport->m_stationIdTable;
    unsigned long long uniqueId = 0;
    transport::Station* station = transport::StationManager::s_pInstance->GetStation(event.stationIndex);
    if (station != 0)
        uniqueId = transport::g_stationConnectionInfoTable->GetPrincipalIdByStation(station);

    switch (event.type)
    {
    case cMeshEventKindJoin:
    {
        transport::StationIdTable::Entry entryByIndex;
        Result findByIndexResult;
        findByIndexResult = stationIdTable->FindByStationIndex(&entryByIndex, event.stationIndex);
        transport::StationIdTable::Entry entryByUniqueId;
        Result findByUniqueIdResult;
        findByUniqueIdResult = stationIdTable->FindByUniqueId(&entryByUniqueId, uniqueId);
        if (findByUniqueIdResult == 0 && event.stationIndex != entryByUniqueId.stationIndex)
        {
            if (!session->m_stationIdStatusTable->CheckConnectStatus(entryByUniqueId.stationId)
                && !session->m_stationIdStatusTable->CheckEventNotifiedStatus(entryByUniqueId.stationId))
            {
                stationIdTable->RemoveByStationId(entryByUniqueId.stationId);
                session->m_stationIdStatusTable->RemoveElement(entryByUniqueId.stationId);
            }
        }

        if (findByIndexResult == 0 && entryByIndex.uniqueId != uniqueId && uniqueId != 0)
        {
            transport::StationIdTable::Entry* entry = stationIdTable->FindByStationIndexCore(event.stationIndex);
            entry->uniqueId = uniqueId;
            entry->stationIndex = event.stationIndex;
            entry->stationId = uniqueId;
            StationIdStatusTable::Element* element = session->m_stationIdStatusTable->FindElement(
                entryByIndex.stationId);
            transport::IdentificationInfo identificationInfo;
            const Result identificationResult =
                transport::IdentificationInfoTable::GetInstance()->GetIdentificationInfo(station, &identificationInfo);
            unsigned int sessionId = 0;
            if (identificationResult == 0)
                sessionId = identificationInfo.m_field10c;
            if (element != 0)
            {
                element->prepare = StationIdStatusTable::cPrepareStatusNone;
                element->answer = StationIdStatusTable::cAnswerStatusNone;
                element->connect = true;
                element->eventNotified = !g_piaSession->IsPostponeNotice();
                element->host = false;
                element->sessionId = sessionId;
                element->stationId = uniqueId;
                element->stationIndex = event.stationIndex;
                element->invitation = false;
                element->migration = false;
            }
            else
            {
                session->m_disconnectStatus = cConnectionStatusDisconnectedSession;
                if (session->m_sessionDisconnectReason == 0)
                    session->m_sessionDisconnectReason = cDisconnectReasonJoinFailed;
                session->CheckStatusAtChangeMeshState();
            }
        }

        if (g_piaMesh->GetLocalStationIndex() == event.stationIndex)
        {
            const unsigned long long localUniqueId = transport::g_stationConnectionInfoTable->GetPrincipalIdByStation(
                transport::StationManager::s_pInstance->m_localStation);
            if (localUniqueId == 0)
                break;
            transport::StationIdTable::Entry entry;
            if (stationIdTable->FindByUniqueId(&entry, localUniqueId) == 0)
            {
                if (entry.stationIndex != event.stationIndex)
                    stationIdTable->FindByStationIdCore(entry.stationId)->stationIndex = event.stationIndex;
                if (session->m_matchmakeStatus != cMatchmakeStatusUnknown3)
                    break;
                session->m_stationIdStatusTable->SetConnect(entry.stationId, true);
            }
            else
            {
                transport::IdentificationInfo identificationInfo;
                const Result identificationResult =
                    transport::IdentificationInfoTable::GetInstance()->GetLocalIdentificationInfo(&identificationInfo);
                unsigned int sessionId = 0;
                if (identificationResult == 0)
                    sessionId = identificationInfo.m_field10c;
                session->AddNewStationId(localUniqueId, event.stationIndex, localUniqueId, sessionId);
            }
        }
        else
        {
            if (station == 0 || uniqueId == 0)
                break;
            bool alreadyJoined = false;
            transport::StationIdTable::Entry entry;
            bool addStation = true;
            if (stationIdTable->FindByUniqueId(&entry, uniqueId) == 0)
            {
                if (session->m_matchmakeStatus == cMatchmakeStatusUnknown3)
                {
                    transport::IdentificationInfo identificationInfo;
                    transport::IdentificationInfoTable::GetInstance()->GetIdentificationInfo(station,
                                                                                             &identificationInfo);
                    unsigned int statusSessionId;
                    session->m_stationIdStatusTable->GetSessionId(entry.stationId, &statusSessionId);
                    if (statusSessionId == identificationInfo.m_field10c)
                    {
                        if (entry.stationIndex != event.stationIndex)
                            stationIdTable->FindByStationIdCore(entry.stationId)->stationIndex = event.stationIndex;
                        session->m_stationIdStatusTable->SetConnect(entry.stationId, true);
                        addStation = false;
                    }
                    else
                    {
                        RemoveStationId(entry.stationId);
                    }
                }
                else
                {
                    if (entry.stationIndex == event.stationIndex)
                    {
                        alreadyJoined = true;
                    }
                    else if (entry.stationIndex != cStationIndexInvalid)
                    {
                        if (session->m_stationIdStatusTable->CheckConnectStatus(entry.stationId))
                            RemoveStationIndex(static_cast<StationIndex>(entry.stationIndex));
                        if (!session->IsProcessingJoin())
                        {
                            session->RemoveStationId(entry.stationId, static_cast<StationIndex>(entry.stationIndex),
                                                     entry.uniqueId, 0);
                        }
                        else
                        {
                            session->LeaveNoticeToWaitingStationIndex(static_cast<StationIndex>(entry.stationIndex));
                            RemoveStationId(entry.stationId);
                        }
                    }
                    if (alreadyJoined)
                        break;
                }
            }
            if (addStation)
            {
                transport::IdentificationInfo identificationInfo;
                transport::IdentificationInfoTable::GetInstance()->GetIdentificationInfo(station, &identificationInfo);
                if (!session->AddNewStationId(uniqueId, event.stationIndex, uniqueId, identificationInfo.m_field10c)
                    && session->m_matchmakeStatus == cMatchmakeStatusUnknown3 && session->m_jointSessionJob != 0
                    && session->m_jointSessionJob->m_processType != 0)
                {
                    session->m_jointSessionJob->m_failurePending = 1;
                    session->m_jointSessionJob->m_pendingFailureResult = cResultOwnerChanged;
                }
            }
        }
        break;
    }
    case cMeshEventKindLeave:
    {
        transport::StationIdTable::Entry entry;
        if (session->m_matchmakeStatus == cMatchmakeStatusUnknown3
            && stationIdTable->FindByStationIndex(&entry, event.stationIndex) == 0)
        {
            session->m_stationIdStatusTable->SetConnect(entry.stationId, false);
            unsigned int statusSessionId;
            session->m_stationIdStatusTable->GetSessionId(entry.stationId, &statusSessionId);
            stationIdTable->FindByStationIndexCore(event.stationIndex)->stationIndex = cStationIndexInvalid;
            if (statusSessionId == session->m_sessionId[session->m_currentSessionIndex])
            {
                if (session->m_fieldc0 != 0)
                {
                    if (session->m_jointSessionJob != 0 && session->m_jointSessionJob->IsRunning())
                        session->m_jointSessionJob->AcceptLeaveStationId(entry.stationId);
                    break;
                }
            }
            else
            {
                RemoveStationId(entry.stationId);
            }

            if (entry.stationId != session->m_hostStationId)
            {
                if (entry.stationId == session->m_jointSessionHostStationId)
                {
                    if (!session->IsHostMigrationEnabled())
                    {
                        if (statusSessionId == session->m_sessionId[session->m_currentSessionIndex])
                        {
                            session->m_disconnectStatus = cConnectionStatusDisconnectedSession;
                            if (session->m_sessionDisconnectReason == 0)
                                session->m_sessionDisconnectReason = cDisconnectReasonJoinFailed;
                            session->CheckStatusAtChangeMeshState();
                        }
                    }
                    else if (session->m_disconnectStatus == 1)
                    {
                        g_piaMesh->GetProcessDestroyMeshJobPtr();
                    }
                }
            }
            else if (!session->IsHostMigrationEnabled())
            {
                session->m_disconnectStatus = cConnectionStatusDisconnectedSession;
                if (session->m_sessionDisconnectReason == 0)
                    session->m_sessionDisconnectReason = cDisconnectReasonJoinFailed;
                session->CheckStatusAtChangeMeshState();
            }
            else if (session->m_disconnectStatus == 1)
            {
                if (!g_piaMesh->GetProcessDestroyMeshJobPtr()->m_started && session->m_jointSessionJob != 0
                    && session->m_jointSessionJob->m_processType == Session::cProcessTypeDestroyJointSession)
                {
                    unsigned int jointHostSessionId;
                    if (!session->m_stationIdStatusTable->GetSessionId(session->m_jointSessionHostStationId,
                                                                       &jointHostSessionId))
                        jointHostSessionId = 0;
                    if (session->m_sessionId[session->m_currentSessionIndex] != jointHostSessionId)
                    {
                        IMatchmakeSession* matchmakeSession =
                            session->m_matchmakeSession[session->m_currentSessionIndex];
                        const unsigned long long hostStationId = session->m_hostStationId;
                        if (matchmakeSession->GetMatchmakeOwnerUniqueId() != hostStationId)
                        {
                            const unsigned long long ownerUniqueId = matchmakeSession->GetMatchmakeOwnerUniqueId();
                            if (ownerUniqueId == session->m_localStationId)
                                matchmakeSession->UpdateMatchmakeSessionHost(
                                    session->m_sessionId[session->m_currentSessionIndex]);
                            session->m_jointSessionJob->NoticeHostChangeEventImpl(ownerUniqueId);
                        }
                    }
                }
            }

            if (entry.stationId == session->m_localStationId)
            {
                session->m_disconnectStatus = cConnectionStatusDisconnectedSession;
                if (session->m_sessionDisconnectReason == 0)
                    session->m_sessionDisconnectReason = cDisconnectReasonJoinFailed;
                session->CheckStatusAtChangeMeshState();
            }
        }
        else
        {
            entry.stationId = 0;
            entry.stationIndex = cStationIndexInvalid;
            transport::Station* leavingStation = transport::StationManager::s_pInstance->GetStation(event.stationIndex);
            if (leavingStation != 0
                && transport::g_stationConnectionInfoTable->GetPrincipalIdByStation(leavingStation) != 0)
                stationIdTable->FindByUniqueId(&entry, uniqueId);
            if (entry.stationIndex != event.stationIndex)
            {
                stationIdTable->FindByStationIndex(&entry, event.stationIndex);
                if (entry.stationIndex != event.stationIndex)
                    break;
            }
            if (!session->IsProcessingJoin())
            {
                const bool postponeNotice = session->m_postponeNotice;
                bool restorePostponeNotice = false;
                if (postponeNotice && session->m_fieldc0 == 0)
                    restorePostponeNotice = true;
                if (restorePostponeNotice)
                    session->m_postponeNotice = false;
                session->RemoveStationId(entry.stationId, static_cast<StationIndex>(entry.stationIndex), entry.uniqueId,
                                         0);
                if (restorePostponeNotice)
                    session->m_postponeNotice = true;
            }
            else
            {
                session->LeaveNoticeToWaitingStationIndex(event.stationIndex);
                RemoveStationId(entry.stationId);
            }

            if (entry.stationId == session->m_hostStationId || entry.stationId == session->m_jointSessionHostStationId)
            {
                if (!session->IsHostMigrationEnabled())
                {
                    session->m_disconnectStatus = cConnectionStatusDisconnectedSession;
                    if (session->m_sessionDisconnectReason == 0)
                        session->m_sessionDisconnectReason = cDisconnectReasonJoinFailed;
                }
                else if (session->m_disconnectStatus == 1)
                {
                    if (!g_piaMesh->GetProcessDestroyMeshJobPtr()->m_started)
                    {
                        unsigned int jointHostSessionId;
                        if (!session->m_stationIdStatusTable->GetSessionId(session->m_jointSessionHostStationId,
                                                                           &jointHostSessionId))
                            jointHostSessionId = 0;
                        const unsigned long long hostStationId = session->m_hostStationId;
                        if (entry.stationId != session->m_jointSessionHostStationId && jointHostSessionId != 0
                            && jointHostSessionId != session->m_sessionId[session->m_currentSessionIndex]
                            && (!g_piaMesh->GetProcessHostMigrationJobPtr()->m_started
                                || (g_piaMesh->GetProcessHostMigrationJobPtr()->m_started
                                    && g_piaMesh->GetProcessHostMigrationJobPtr()->m_unknownB8)))
                        {
                            const unsigned long long ownerUniqueId =
                                session->m_matchmakeSession[session->m_currentSessionIndex]
                                    ->GetMatchmakeOwnerUniqueId();
                            if (ownerUniqueId != hostStationId)
                            {
                                session->m_hostStationId = ownerUniqueId;
                                session->CallSessionEventCallback(Session::cEventTypeUnknown02,
                                                                  session->m_hostStationId);
                            }
                        }
                    }
                }
            }
            if (entry.stationId == session->m_localStationId)
            {
                session->m_disconnectStatus = cConnectionStatusDisconnectedSession;
                if (session->m_sessionDisconnectReason == 0)
                    session->m_sessionDisconnectReason = cDisconnectReasonJoinFailed;
                session->CheckStatusAtChangeMeshState();
            }
        }
        break;
    }
    case cMeshEventKind2:
        if (!session->IsProcessingJoin() && !session->m_leaveSessionJob->IsRunning()
            && session->m_disconnectStatus - cConnectionStatusDisconnectedSession > 4)
        {
            const unsigned int matchmakeStatus = session->m_matchmakeStatus;
            if (matchmakeStatus == cMatchmakeStatusUnknown3)
            {
                transport::StationIdTable::Entry entry;
                if (stationIdTable->FindByStationIndex(&entry, event.stationIndex) == 0
                    && session->m_jointSessionJob != 0 && session->m_jointSessionJob->m_processType != 0)
                {
                    session->m_jointSessionJob->NoticeHostChangeEventImpl(entry.stationId);
                    if (session->m_localStationId == entry.stationId)
                    {
                        IMatchmakeSession* matchmakeSession =
                            session->m_matchmakeSession[session->m_currentSessionIndex];
                        if (!matchmakeSession->IsMatchmakeOwner())
                            matchmakeSession->UpdateMatchmakeSessionHost(
                                session->m_sessionId[session->m_currentSessionIndex]);
                    }
                }
            }
            else
            {
                bool jointHostChanged = false;
                bool hostChanged = false;
                transport::StationIdTable::Entry entry;
                if (matchmakeStatus == cMatchmakeStatusUnknown2)
                {
                    if (stationIdTable->FindByUniqueId(&entry, uniqueId) == 0)
                    {
                        session->m_hostStationId = entry.stationId;
                        hostChanged = true;
                        if (session->m_localStationId == entry.stationId
                            && !session->m_matchmakeSession[session->m_currentSessionIndex]->IsMatchmakeOwner())
                            hostChanged = false;
                    }
                }
                else if (matchmakeStatus == cMatchmakeStatusUnknown4
                         && stationIdTable->FindByUniqueId(&entry, uniqueId) == 0)
                {
                    session->m_jointSessionHostStationId = entry.stationId;
                    unsigned int hostSessionId;
                    session->m_stationIdStatusTable->GetSessionId(entry.stationId, &hostSessionId);
                    session->m_field110 = hostSessionId;
                    const unsigned long long jointHostStationId = session->m_jointSessionHostStationId;
                    const unsigned char bufferIndex = session->m_currentSessionIndex == 0 ? 1 : 0;
                    jointHostChanged = true;
                    if (jointHostStationId == session->m_localStationId)
                    {
                        jointHostChanged =
                            session->m_matchmakeSession[bufferIndex]->GetMatchmakeOwnerUniqueId() == jointHostStationId
                            && session->m_matchmakeSession[session->m_currentSessionIndex]->GetMatchmakeOwnerUniqueId()
                                   == session->m_jointSessionHostStationId;
                    }
                    if (session->m_field110 != session->m_sessionId[session->m_currentSessionIndex])
                    {
                        transport::StationIdTable::Entry hostEntry;
                        if (transport::g_piaTransport->m_stationIdTable->FindByStationId(&hostEntry,
                                                                                         session->m_hostStationId)
                            != 0)
                        {
                            IMatchmakeSession* matchmakeSession =
                                session->m_matchmakeSession[session->m_currentSessionIndex];
                            const unsigned long long oldHostStationId = session->m_hostStationId;
                            if (matchmakeSession->GetMatchmakeOwnerUniqueId() != oldHostStationId)
                            {
                                const unsigned long long newHostStationId =
                                    matchmakeSession->GetMatchmakeOwnerUniqueId();
                                session->m_hostStationId = newHostStationId;
                                if (newHostStationId == session->m_localStationId)
                                    matchmakeSession->UpdateMatchmakeSessionHost(
                                        session->m_sessionId[session->m_currentSessionIndex]);
                                hostChanged = true;
                            }
                        }
                    }
                    else if (session->m_hostStationId != entry.stationId)
                    {
                        if (session->m_hostStationId != session->m_localStationId)
                        {
                            StationIndex hostStationIndex;
                            if (transport::g_piaTransport->ConvertToStationIndex(&hostStationIndex,
                                                                                 session->m_hostStationId)
                                    == 0
                                && g_piaMesh->IsValidStationIndex(hostStationIndex))
                                RemoveStationIndex(hostStationIndex);
                            session->m_hostStationId = entry.stationId;
                            hostChanged = true;
                            if (entry.stationId == session->m_localStationId)
                            {
                                IMatchmakeSession* matchmakeSession =
                                    session->m_matchmakeSession[session->m_currentSessionIndex];
                                matchmakeSession->UpdateMatchmakeSessionHost(
                                    session->m_sessionId[session->m_currentSessionIndex]);
                                const unsigned long long hostStationId = session->m_hostStationId;
                                hostChanged = matchmakeSession->GetMatchmakeOwnerUniqueId() == hostStationId;
                            }
                        }
                        else
                        {
                            session->m_disconnectStatus = cConnectionStatusDisconnectedSession;
                            if (session->m_sessionDisconnectReason != cLeaveSessionDisconnectReason)
                                session->m_sessionDisconnectReason = cDisconnectReasonOwnerChanged;
                            session->CheckStatusAtChangeMeshState();
                        }
                    }
                }
                if (jointHostChanged)
                    session->CallSessionEventCallback(Session::cEventTypeUnknown04,
                                                      session->m_jointSessionHostStationId);
                if (hostChanged)
                    session->CallSessionEventCallback(Session::cEventTypeUnknown02, session->m_hostStationId);
                if (g_piaMesh != 0 && g_piaMesh->GetProcessHostMigrationJobPtr() != 0
                    && g_piaMesh->GetProcessHostMigrationJobPtr()->m_started
                    && ((session->m_matchmakeStatus == cMatchmakeStatusUnknown2 && hostChanged)
                        || (session->m_matchmakeStatus == cMatchmakeStatusUnknown4 && jointHostChanged)))
                {
                    session->m_postponeNotice = false;
                    session->m_stationIdStatusTable->ClearDisconnectedStationId();
                    session->m_stationIdStatusTable->CallJoinEventNotYetNotified();
                }
            }
        }
        break;
    case cMeshEventKind3:
        if (session->m_matchmakeStatus == cMatchmakeStatusUnknown3)
        {
            if (session->m_jointSessionJob != 0)
            {
                session->m_jointSessionJob->m_failurePending = 1;
                session->m_jointSessionJob->m_pendingFailureResult = cResultOwnerChanged;
            }
        }
        else
        {
            session->m_disconnectStatus = cConnectionStatusDisconnectedSession;
            if (session->m_sessionDisconnectReason != cLeaveSessionDisconnectReason)
                session->m_sessionDisconnectReason = cDisconnectReasonJointSession;
            session->m_hostStationId = 0;
            session->m_jointSessionHostStationId = 0;
            session->CallSessionEventCallback(Session::cEventTypeUnknown03, 0);
        }
        session->m_postponeNotice = false;
        break;
    case cMeshEventKind4:
        session->m_postponeNotice = true;
        break;
    case cMeshEventKindCheckStatus:
        session->CheckStatusAtChangeMeshState();
        break;
    case cMeshEventKind21:
    {
        const StationIndex hostStationIndex = g_piaMesh->GetHostStationIndex();
        if (session->m_matchmakeSession[session->m_currentSessionIndex]->GetMatchmakeOwnerUniqueId() == uniqueId
            && hostStationIndex != event.stationIndex)
        {
            transport::Station* hostStation = transport::StationManager::s_pInstance->GetStation(hostStationIndex);
            if (hostStation != 0)
            {
                if (hostStation->m_field20 == cStationLeaveStateRequested)
                    hostStation->m_field68 = 1;
                else
                    hostStation->m_field20 = cStationLeaveStateLeaving;
            }
        }
        break;
    }
    case cMeshEventKind23:
    {
        const unsigned char playerNum = static_cast<unsigned char>(event.stationId);
        const unsigned short availableNum = static_cast<unsigned short>((event.stationId >> 8) & 0xff);
        if (session->m_jointSessionJob == 0 || !session->m_jointSessionJob->IsRunning())
        {
            session->m_fieldc2[session->m_currentSessionIndex] = availableNum;
            transport::g_piaTransport->m_stationIdTable->SetAvailableEntryNum(
                g_piaSession->m_fieldc2[g_piaSession->m_currentSessionIndex]);
        }
        else
        {
            if (session->m_fieldc2[session->m_currentSessionIndex] == 0)
                session->m_fieldc2[session->m_currentSessionIndex] = availableNum;
            session->m_fieldc2[session->m_currentSessionIndex == 0 ? 1 : 0] = playerNum;
            const Session::ProcessType processType = session->m_jointSessionJob->m_processType;
            if (processType == Session::cProcessTypeCreateJointSession
                || processType == Session::cProcessTypeJoinJointSession
                || processType == Session::cProcessTypeJoinRandomJointSession)
                transport::g_piaTransport->m_stationIdTable->SetAvailableEntryNum(
                    g_piaSession->m_fieldc2[g_piaSession->m_currentSessionIndex == 0 ? 1 : 0]);
            else
                transport::g_piaTransport->m_stationIdTable->SetAvailableEntryNum(
                    g_piaSession->m_fieldc2[g_piaSession->m_currentSessionIndex]);
        }
        const unsigned int stationNum = (event.stationId >> 16) & 0xff;
        if (stationNum == 0 || transport::g_piaTransport->m_stationNum == stationNum)
            break;
        session->CancelJoinProcessByFatalError();
        break;
    }
    case cMeshEventKind24:
        if (session->m_matchmakeStatus == cMatchmakeStatusUnknown2)
        {
            unsigned int participantCount = 0;
            const common::ListNode* sentinel = session->m_validStationIdList.Sentinel();
            for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
            {
                transport::Station* joiningStation = transport::StationManager::s_pInstance->GetStation(
                    *Session::StationIdList::ValueOf(node));
                unsigned char count;
                if (joiningStation != 0 && joiningStation->GetParticipantCount(&count) == 0)
                    participantCount = (participantCount + count) & 0xffff;
            }
            if (session->m_fieldc2[session->m_currentSessionIndex] < participantCount + (event.stationId & 0xffff))
                g_piaMesh->SetDenyJoinRequestBySessionIsFull(true);
        }
        break;
    case cMeshEventKind25:
    {
        bool useCurrentSession = true;
        if (session->GetStatus() != cStatusMatchedCurrent)
        {
            useCurrentSession = false;
            if (session->GetStatus() != cStatusMatchedBuffer)
            {
                if (session->GetStatus() != cStatusUnknown2)
                    break;
                useCurrentSession = session->GetJointSessionProcessType() != Session::cProcessTypeCreateJointSession
                                    && session->GetJointSessionProcessType() != Session::cProcessTypeJoinJointSession
                                    && session->GetJointSessionProcessType()
                                           != Session::cProcessTypeJoinRandomJointSession;
            }
        }
        const unsigned int sessionId = useCurrentSession
                                           ? session->m_sessionId[session->m_currentSessionIndex]
                                           : session->m_sessionId[session->m_currentSessionIndex == 0 ? 1 : 0];
        if (sessionId != 0 && !session->m_meshLayerController->IsParticipated(sessionId, event.stationId))
            g_piaMesh->SetDenyJoinRequestByInvalidState(true);
        break;
    }
    }

    stationIdTable->Trace(cTraceMaskStationTable);
    session->m_stationIdStatusTable->Trace(cTraceMaskStationTable);
    if (Session::s_debugSetting.m_matchmakeAnalysisEnabled != 0)
    {
        if (event.type == cMeshEventKind2)
            session->SetHostMigrationOccurred(true);
    }

    if (session->m_joinSessionJob != 0 && session->m_joinSessionJob->IsRunning())
    {
        if (event.type == cMeshEventKind19)
            session->m_joinSessionJob->m_sessionInfoRetry = true;
        if (event.type == cMeshEventKind20)
            session->m_joinSessionJob->m_retryReasonB = true;
        if (event.type == cMeshEventKind22)
            session->m_joinSessionJob->m_retryReasonA = true;
    }
    if (session->m_randomMatchmakeJob != 0 && session->m_randomMatchmakeJob->IsRunning())
    {
        if (event.type == cMeshEventKind19)
            session->m_randomMatchmakeJob->m_sessionInfoRetry = true;
        if (event.type == cMeshEventKind20)
            session->m_randomMatchmakeJob->m_retryReasonB = true;
        if (event.type == cMeshEventKind22)
            session->m_randomMatchmakeJob->m_retryReasonA = true;
    }
    if (session->m_jointSessionJob != 0 && session->m_jointSessionJob->IsRunning())
    {
        if (event.type == cMeshEventKind19)
            session->m_jointSessionJob->m_sessionInfoRetry = 1;
        if (event.type == cMeshEventKind20)
            session->m_jointSessionJob->m_retryReasonB = 1;
    }
}

}
}
}
