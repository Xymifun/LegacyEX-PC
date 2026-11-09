#include "nn/pia/session/CreateMeshJob.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/ProcessJoinRequestJob.hpp"
#include "nn/pia/session/SyncClockProtocol.hpp"
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
typedef common::Job::ExecuteResult (CreateMeshJob::*Step)();

const unsigned int cMonitoringValueUnset = 0xffffffffu;

enum
{
    cResultSetupLocalPlayerInfoFailed = 0x6c65,
    cResultInvalidState = 0x10c08,
    cConnectingStationState = 2,
    cConnectedStationState = 5
};

inline void SelectStep(common::StepSequenceJob::Step& slot, Step function, const char* name)
{
    const common::StepSequenceJob::Step step(
        static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

CreateMeshJob::CreateMeshJob() : common::StepSequenceJob(), m_callerContext(0)
{
}

CreateMeshJob::~CreateMeshJob()
{
}

void CreateMeshJob::SetHostInfoToMonitoringData()
{
    SetHostInfoToMonitoringDataImpl();
}

Result CreateMeshJob::Startup(common::CallContext* callerContext)
{
    if (callerContext != 0)
    {
        m_callerContext = callerContext;
        callerContext->InitiateCall();
    }

    Result result = StartupImpl();
    if (result == 0)
    {
        g_piaMesh->CleanupMonitoringBeginData();
        SetHostInfoToMonitoringData();
    }
    return result;
}

CreateMeshJob::ExecuteResult CreateMeshJob::SetupLocalPlayerInfo()
{
    if (g_piaMesh->SetupLocalPlayerInfo() != 0)
        SelectStep(m_currentStep, &CreateMeshJob::CompleteFailure, "CreateMeshJob::CompleteFailure");
    else
        SetNextStepAtSetupLocalPlayerInfoImpl();
    return MakeExecuteResult(cExecuteKindEntryForeground);
}

void CreateMeshJob::Cleanup()
{
    if (IsRunning() && IsBackground())
    {
        m_cancelRequested = true;
        do
        {
            WaitForCompletion(2);
        } while (IsRunning());
    }
    CleanupImpl();
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
    }
}

CreateMeshJob::ExecuteResult CreateMeshJob::CompleteFailure()
{
    if (m_callerContext != 0)
    {
        m_callerContext->SignalFailure(cResultSetupLocalPlayerInfoFailed);
        m_callerContext = 0;
    }
    Cleanup();
    return MakeExecuteResult(cExecuteKindFinished);
}

CreateMeshJob::ExecuteResult CreateMeshJob::SetupLocalStation()
{
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }

    Result result = g_piaMesh->SetupProtocols();
    if (result != 0)
    {
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(result);
            m_callerContext = 0;
        }
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }

    g_piaMesh->ResetStationIndexUsage();
    g_piaMesh->m_unknown58 = g_piaMesh->m_unknown5c = g_piaMesh->GetUsableStationIndex();
    g_piaMesh->StartUse(g_piaMesh->m_unknown5c);

    transport::Station* station = transport::StationManager::s_pInstance->CreateLocalStation();
    transport::StationConnectionInfoTable* table = transport::g_stationConnectionInfoTable;
    const unsigned long long principalId = table->m_localStationConnectionInfo.m_stationLocation.m_principalId;
    const common::StationAddress address = table->m_localStationConnectionInfo.m_stationLocation.m_stationAddress;
    if (!station->Startup(principalId, g_piaMesh->m_unknown5c, address))
    {
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(cResultInvalidState);
            m_callerContext = 0;
        }
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }

    result = table->AddToTable(station, table->m_localStationConnectionInfo);

    transport::IdentificationInfoTable* identificationInfoTable = transport::IdentificationInfoTable::GetInstance();
    transport::IdentificationInfo info;
    result = identificationInfoTable->GetLocalIdentificationInfo(&info);
    if (g_piaMesh->m_getSessionIdCallback != 0)
        info.m_field10c = g_piaMesh->m_getSessionIdCallback();
    else
        info.m_field10c = 0;
    info.m_playerCount = static_cast<unsigned char>(g_piaMesh->GetLocalPlayerCount());
    info.m_participantCount = g_piaMesh->IsPlayersCountedAsParticipants() ? info.m_playerCount : 1;
    transport::IdentificationInfoTable::GetInstance()->SetLocalIdentificationInfo(&info);
    result = transport::IdentificationInfoTable::GetInstance()->AddToTable(station, &info, 0);
    if (result != 0)
    {
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(result);
            m_callerContext = 0;
        }
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }

    transport::StationManager::s_pInstance->m_invalidStationIndex = g_piaMesh->m_unknown58;
    result = transport::g_piaTransport->m_protocolManager.StartupProtocolsWithStationAddress(station->m_stationAddress);
    if (result != 0)
    {
        transport::g_stationConnectionInfoTable->EraseFromTable(station);
        transport::IdentificationInfoTable::GetInstance()->EraseFromTable(station);
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(result);
            m_callerContext = 0;
        }
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }

    station->m_field20 = cConnectingStationState;
    g_piaMesh->m_unknown80 = 0;
    g_piaMesh->m_unknownc8 = 0;
    g_piaMesh->m_unknowncc = 0;
    if (g_piaMesh->m_unknown0c->Startup())
        g_piaMesh->m_unknown0c->Ready(false);
    table->m_hostStationConnectionInfo = table->m_localStationConnectionInfo;
    station->m_field20 = cConnectedStationState;
    g_piaMesh->SetMeshState(Mesh::cStateCreated);
    g_piaMesh->m_disconnectReason = 0;
    g_piaMesh->m_unknown56 = 1;
    g_piaMesh->NoticeMeshEvent(Mesh::cEventTypeJoin, g_piaMesh->m_unknown5c);

    SyncClockProtocol* syncClockProtocol = static_cast<SyncClockProtocol*>(
        transport::g_piaTransport->m_protocolManager.SearchProtocol(g_piaMesh->m_syncClockProtocolId,
                                                                    SyncClockProtocol::cProtocolType));
    common::Time now;
    now.SetNow();
    syncClockProtocol->m_syncClock.SetBaseTime(now);
    g_piaMesh->SetMeshStartTime(now);

    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
            m_callerContext->SignalSuccess(0);
        m_callerContext = 0;
    }

    common::g_sessionBeginMonitoringContent.m_joinResult = cMonitoringValueUnset;
    common::g_sessionBeginMonitoringContent.m_metric47 = static_cast<unsigned char>(g_piaMesh->GetStationNum());
    common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = cMonitoringValueUnset;
    g_piaMesh->ProcessingAtMeshBegin();
    return MakeExecuteResult(cExecuteKindFinished);
}

void CreateMeshJob::Trace(unsigned long long) const
{
}

Result CreateMeshJob::StartupImpl()
{
    Reset(true);
    SelectStep(m_currentStep, &CreateMeshJob::SetupLocalPlayerInfo, "CreateMeshJob::SetupLocalPlayerInfo");
    return 0;
}

void CreateMeshJob::SetNextStepAtSetupLocalPlayerInfoImpl()
{
    SelectStep(m_currentStep, &CreateMeshJob::SetupLocalStation, "CreateMeshJob::SetupLocalStation");
}

void CreateMeshJob::CleanupImpl()
{
}

void CreateMeshJob::SetHostInfoToMonitoringDataImpl()
{
}

}
}
}
