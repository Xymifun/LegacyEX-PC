#include "nn/pia/session/ProcessJoinRequestJob.hpp"

#include <new>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshProtocol.hpp"
#include "nn/pia/transport/DisconnectStationJob.hpp"
#include "nn/pia/transport/IdentificationInfoTable.hpp"
#include "nn/pia/transport/RelayRouteManager.hpp"
#include "nn/pia/transport/ResendingMessageManager.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "platform/wiiu/pead.hpp"

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
    cStationIndexInvalid = 0xfd,
    cResponseBufferSize = 0x59c,
    cResponseBufferAlignment = 8,
    cStationsPerPartWithFlag = 6,
    cStationsPerPart = 16,
    cDefaultTimeoutMsec = 10000,
    cApprovedResult = 0xff,
    cRefusedJoinCountWrap = 0xff,
    cDenyReasonNone = 3,
    cDenyReasonRefused = 1,
    cConnectedStationState = 5,
    cStationState6 = 6,
    cTraceStationLevel = 0x8000000
};

unsigned char s_refusedJoinCount;
}

ProcessJoinRequestJob::ProcessJoinRequestJob()
    : common::StepSequenceJob(),
      m_deadline(),
      m_timeoutMsec(cDefaultTimeoutMsec),
      m_unknown4c(0),
      m_joiningStationAddress(),
      m_stationIndex(static_cast<StationIndex>(cStationIndexInvalid)),
      m_unknown78(false),
      m_denyReason(cDenyReasonNone),
      m_joinCancelRequested(false)
{
    const unsigned int perPart = g_piaMesh->m_unknown75 != 0 ? cStationsPerPartWithFlag : cStationsPerPart;
    const unsigned char partCount = static_cast<unsigned char>(
        (static_cast<int>(g_piaMesh->m_unknown54) + static_cast<int>(perPart) - 1) / static_cast<int>(perPart));
    for (unsigned int part = 0; part < cResponsePartMax; ++part)
    {
        if (part < partCount)
            m_responseBuffers[part] = common::NewArray<unsigned char>(cResponseBufferSize, cResponseBufferAlignment);
        else
            m_responseBuffers[part] = 0;
    }
    for (unsigned int part = 0; part < cResponsePartMax; ++part)
        m_responseAckIds[part] = 0;
}

ProcessJoinRequestJob::~ProcessJoinRequestJob()
{
    for (unsigned int part = 0; part < cResponsePartMax; ++part)
        common::DeleteArray(m_responseBuffers[part]);
}

void ProcessJoinRequestJob::Trace(unsigned long long) const
{
}

bool ProcessJoinRequestJob::Startup()
{
    g_piaMesh->GetMeshProtocol()->SetWaitingJoinRequestJob(this);
    m_unknown4c = 0;
    for (unsigned int part = 0; part < cResponsePartMax; ++part)
        m_responseAckIds[part] = 0;
    m_stationIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_joinCancelRequested = false;
    Reset(true);
    setStep(&ProcessJoinRequestJob::InitialStep, "ProcessJoinRequestJob::InitialStep");
    return true;
}

ProcessJoinRequestJob::ExecuteResult ProcessJoinRequestJob::InitialStep()
{
    m_unknown4c = 0;
    for (int part = 0; part < cResponsePartMax; ++part)
        m_responseAckIds[part] = 0;
    m_unknown78 = false;
    m_denyReason = cDenyReasonNone;
    m_stationIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_joinCancelRequested = false;
    setStep(&ProcessJoinRequestJob::CheckApprovalJoin, "ProcessJoinRequestJob::CheckApprovalJoin");
    return MakeExecuteResult(cExecuteKindSuspend);
}

ProcessJoinRequestJob::ExecuteResult ProcessJoinRequestJob::CheckApprovalJoin()
{
    m_joiningStationAddress.Trace(cTraceStationLevel);
    m_unknown78 = true;
    transport::Station* station = transport::StationManager::s_pInstance->GetStation(m_joiningStationAddress);
    if (station == 0)
    {
        setStep(&ProcessJoinRequestJob::InitialStep, "ProcessJoinRequestJob::InitialStep");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (station->m_field20 != cConnectedStationState)
    {
        setStep(&ProcessJoinRequestJob::InitialStep, "ProcessJoinRequestJob::InitialStep");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    Result approval = g_piaMesh->CheckApprovalJoin(station);
    if (approval != cApprovedResult)
    {
        m_denyReason = static_cast<unsigned char>(approval.m_code);
        setStep(&ProcessJoinRequestJob::SendDenyingJoinResponse, "ProcessJoinRequestJob::SendDenyingJoinResponse");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (g_piaMesh->m_joiningMeshApprovalCallback == 0)
    {
        setStep(&ProcessJoinRequestJob::SendJoinResponse, "ProcessJoinRequestJob::SendJoinResponse");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    {
        transport::IdentificationInfo identificationInfo;
        Result identificationResult = transport::IdentificationInfoTable::GetInstance()->GetIdentificationInfo(
            station, &identificationInfo);
        if (identificationResult != 0)
        {
            setStep(&ProcessJoinRequestJob::InitialStep, "ProcessJoinRequestJob::InitialStep");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (!g_piaMesh->m_joiningMeshApprovalCallback(&identificationInfo.m_field00))
        {
            m_denyReason = cDenyReasonRefused;
            s_refusedJoinCount = s_refusedJoinCount == cRefusedJoinCountWrap ? 1 : s_refusedJoinCount + 1;
            setStep(&ProcessJoinRequestJob::SendDenyingJoinResponse, "ProcessJoinRequestJob::SendDenyingJoinResponse");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    setStep(&ProcessJoinRequestJob::SendJoinResponse, "ProcessJoinRequestJob::SendJoinResponse");
    return MakeExecuteResult(cExecuteKindContinue);
}

ProcessJoinRequestJob::ExecuteResult ProcessJoinRequestJob::SendDenyingJoinResponse()
{
    if (transport::StationManager::s_pInstance->m_localStation == 0)
    {
        setStep(&ProcessJoinRequestJob::InitialStep, "ProcessJoinRequestJob::InitialStep");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    g_piaMesh->GetMeshProtocol()->SendDenyingJoinResponse(m_joiningStationAddress, m_denyReason);
    transport::Station* station = transport::StationManager::s_pInstance->GetStation(m_joiningStationAddress);
    if (station != 0)
        station->m_field20 = cStationState6;
    setStep(&ProcessJoinRequestJob::InitialStep, "ProcessJoinRequestJob::InitialStep");
    return MakeExecuteResult(cExecuteKindContinue);
}

ProcessJoinRequestJob::ExecuteResult ProcessJoinRequestJob::SendJoinResponse()
{
    if (transport::StationManager::s_pInstance->m_localStation == 0)
    {
        setStep(&ProcessJoinRequestJob::InitialStep, "ProcessJoinRequestJob::InitialStep");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    transport::Station* station = transport::StationManager::s_pInstance->GetStation(m_joiningStationAddress);
    if (station == 0)
    {
        setStep(&ProcessJoinRequestJob::InitialStep, "ProcessJoinRequestJob::InitialStep");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    m_stationIndex = g_piaMesh->GetUsableStationIndex();
    if (m_stationIndex == cStationIndexInvalid)
    {
        setStep(&ProcessJoinRequestJob::SendDenyingJoinResponse, "ProcessJoinRequestJob::SendDenyingJoinResponse");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    transport::Station* staleStations[transport::StationManager::cStationNumMax] = {};
    unsigned short staleCount = 0;
    unsigned long long stationId = 0;
    {
        transport::StationConnectionInfo joiningInfo;
        transport::StationConnectionInfo otherInfo;
        Result result = transport::g_stationConnectionInfoTable->GetStationConnectionInfo(station, &joiningInfo);
        if (result == 0)
        {
            stationId = joiningInfo.m_stationLocation.m_principalId;
            transport::Station** current = transport::StationManager::s_pInstance->m_stations.data;
            if (joiningInfo.m_stationLocation.m_stationAddress.IsValid() != 0
                && current != current + transport::StationManager::s_pInstance->m_stations.size)
            {
                do
                {
                    if (*current != station)
                    {
                        result = transport::g_stationConnectionInfoTable->GetStationConnectionInfo(*current,
                                                                                                   &otherInfo);
                        if (result == 0
                            && joiningInfo.m_stationLocation.m_field1c == otherInfo.m_stationLocation.m_field1c)
                        {
                            staleStations[staleCount] = *current;
                            ++staleCount;
                        }
                    }
                    ++current;
                } while (current
                         != transport::StationManager::s_pInstance->m_stations.data
                                + transport::StationManager::s_pInstance->m_stations.size);
                if (staleCount != 0)
                {
                    bool meshChanged = false;
                    for (unsigned short index = 0; index < staleCount; ++index)
                    {
                        transport::Station* stale = staleStations[index];
                        stale->Trace(cTraceStationLevel);
                        const StationIndex staleIndex = static_cast<StationIndex>(stale->m_field10);
                        if (g_piaMesh->IsValidStationIndex(staleIndex))
                        {
                            g_piaMesh->UnfixDisconnectedId(staleIndex);
                            meshChanged = true;
                        }
                        stale->m_disconnectStationJob->ProcessDisconnection(stale);
                        transport::g_stationConnectionInfoTable->EraseFromTable(stale);
                        transport::IdentificationInfoTable::GetInstance()->EraseFromTable(stale);
                        stale->Cleanup();
                        stale->CleanupJobs();
                        transport::StationManager::s_pInstance->DestroyStation(stale);
                    }
                    transport::g_piaTransport->OutputStreamUpdateEvent();
                    if (meshChanged)
                        g_piaMesh->GetMeshProtocol()->SendUpdateMesh(true, 1);
                }
            }
        }
    }
    station->SetStationIdAndIndex(stationId, m_stationIndex);
    g_piaMesh->FixConnectedId(m_stationIndex);
    transport::RelayRouteManager* relayRouteManager = transport::g_piaTransport->m_relayRouteManager;
    if (relayRouteManager != 0)
        relayRouteManager->SetRelayRoute(g_piaMesh->GetLocalStationIndex(), m_stationIndex, m_stationIndex);
    common::Time now;
    now.SetNow();
    m_deadline.m_tick = now.m_tick + m_timeoutMsec * common::TimeSpan::GetTicksPerMSec().m_tick;
    unsigned char* messages[cResponsePartMax] = {m_responseBuffers[0], m_responseBuffers[1], m_responseBuffers[2]};
    unsigned int sizes[cResponsePartMax] = {};
    const unsigned int partCount = g_piaMesh->GetMeshProtocol()->MakeJoinResponseData(m_stationIndex, messages, sizes);
    if (partCount < 1 || partCount > cResponsePartMax)
    {
        setStep(&ProcessJoinRequestJob::InitialStep, "ProcessJoinRequestJob::InitialStep");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    for (unsigned char part = 0; part < partCount; ++part)
    {
        Result sendResult = transport::g_piaResendingMessageManager->SetSendMessageByStationAddress(
            &m_responseAckIds[part], messages[part], sizes[part], station->m_stationAddress,
            g_piaMesh->GetMeshProtocol()->mProtocolId, m_deadline.m_tick);
        if (sendResult != 0)
        {
            for (unsigned int stopped = part; stopped != 0; --stopped)
                transport::g_piaResendingMessageManager->StopResending(m_responseAckIds[stopped - 1]);
            setStep(&ProcessJoinRequestJob::InitialStep, "ProcessJoinRequestJob::InitialStep");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    for (unsigned int part = partCount; part < cResponsePartMax; ++part)
        m_responseAckIds[part] = 0;
    setStep(&ProcessJoinRequestJob::WaitResponseAck, "ProcessJoinRequestJob::WaitResponseAck");
    return MakeExecuteResult(cExecuteKindNextStep);
}

ProcessJoinRequestJob::ExecuteResult ProcessJoinRequestJob::WaitResponseAck()
{
    bool resending = false;
    for (unsigned int part = 0; part < cResponsePartMax; ++part)
        resending |= transport::g_piaResendingMessageManager->CheckNowResending(m_responseAckIds[part]);
    if (resending)
    {
        common::Time now;
        now.SetNow();
        if (m_deadline <= now || m_joinCancelRequested)
        {
            for (unsigned int part = 0; part < cResponsePartMax; ++part)
            {
                transport::g_piaResendingMessageManager->StopResending(m_responseAckIds[part]);
                m_responseAckIds[part] = 0;
            }
            m_deadline.m_tick = 0;
            setStep(&ProcessJoinRequestJob::InitialStep, "ProcessJoinRequestJob::InitialStep");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    setStep(&ProcessJoinRequestJob::JoinSucceeded, "ProcessJoinRequestJob::JoinSucceeded");
    return MakeExecuteResult(cExecuteKindContinue);
}

ProcessJoinRequestJob::ExecuteResult ProcessJoinRequestJob::JoinSucceeded()
{
    if (!m_joinCancelRequested)
    {
        g_piaMesh->JoinResponseComplete(m_stationIndex);
        g_piaMesh->GetMeshProtocol()->SendUpdateMesh(true, 1);
    }
    setStep(&ProcessJoinRequestJob::InitialStep, "ProcessJoinRequestJob::InitialStep");
    return MakeExecuteResult(cExecuteKindContinue);
}

void ProcessJoinRequestJob::SetJoiningStationData(const common::StationAddress& stationAddress)
{
    m_joiningStationAddress = stationAddress;
}

void ProcessJoinRequestJob::CancellationNotice(StationIndex stationIndex)
{
    if (m_stationIndex == stationIndex)
        m_joinCancelRequested = true;
}

void ProcessJoinRequestJob::Cleanup()
{
    if (transport::g_piaResendingMessageManager != 0)
    {
        if (m_unknown4c != 0)
            transport::g_piaResendingMessageManager->StopResending(m_unknown4c);
        for (unsigned int part = 0; part < cResponsePartMax; ++part)
            transport::g_piaResendingMessageManager->StopResending(m_responseAckIds[part]);
    }
    m_unknown4c = 0;
    for (unsigned int part = 0; part < cResponsePartMax; ++part)
        m_responseAckIds[part] = 0;
    m_unknown78 = false;
    m_joinCancelRequested = false;
    m_stationIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_denyReason = cDenyReasonNone;
    m_deadline.m_tick = 0;
}

}
}
}
