#include "nn/pia/transport/DisconnectStationJob.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/transport/ConnectStationJob.hpp"
#include "nn/pia/transport/IdentificationInfoTable.hpp"
#include "nn/pia/transport/ProcessConnectionRequestJob.hpp"
#include "nn/pia/transport/RelayRouteManager.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationProtocol.hpp"

namespace nn
{
namespace pia
{
namespace transport
{
namespace
{
typedef common::Job::ExecuteResult (DisconnectStationJob::*Step)();

inline void SelectStep(common::StepSequenceJob::Step& slot, Step function, const char* name)
{
    slot = common::StepSequenceJob::Step(
        static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
}
}

DisconnectStationJob::DisconnectStationJob()
    : common::StepSequenceJob(),
      m_deadline(),
      m_timeoutMsec(4000),
      m_station(0),
      m_retryDeadline(),
      m_retryIntervalMsec(500),
      m_waiting(false)
{
}

DisconnectStationJob::~DisconnectStationJob()
{
}

Result DisconnectStationJob::Startup(Station* station)
{
    return StartupImpl(station);
}

DisconnectStationJob::ExecuteResult DisconnectStationJob::SendDisconnectionRequest()
{
    if (m_station->GetStationIndex() <= StationManager::cStationNumMax - 1)
    {
        m_station->m_field28->SendDisconnectionRequest(m_station->GetStationIndex(), false);
    }
    else
    {
        if (m_station->GetStationAddress().IsValid())
        {
            m_station->m_field28->SendDisconnectionRequest(m_station->GetStationAddress());
        }
        else
        {
            SelectStep(m_currentStep, &DisconnectStationJob::DisconnectionSucceeded,
                       "DisconnectStationJob::DisconnectionSucceeded");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    const common::TimeSpan retryInterval = common::TimeSpan::FromMSec(m_retryIntervalMsec);
    m_retryDeadline = common::g_piaJobScheduler->m_dispatchTime + retryInterval;
    m_waiting = true;
    SelectStep(m_currentStep, &DisconnectStationJob::WaitForDisconnection,
               "DisconnectStationJob::WaitForDisconnection");
    return MakeExecuteResult(cExecuteKindNextStep);
}

DisconnectStationJob::ExecuteResult DisconnectStationJob::WaitForDisconnection()
{
    if (m_waiting)
    {
        if (m_deadline <= common::g_piaJobScheduler->m_dispatchTime
            || (m_station->GetStationIndex() > StationManager::cStationNumMax - 1
                && !m_station->GetStationAddress().IsValid()))
        {
            m_waiting = false;
        }
        else
        {
            if (m_retryDeadline <= common::g_piaJobScheduler->m_dispatchTime)
            {
                if (m_station->GetStationIndex() <= StationManager::cStationNumMax - 1)
                {
                    m_station->m_field28->SendDisconnectionRequest(m_station->GetStationIndex(), false);
                }
                else
                {
                    m_station->m_field28->SendDisconnectionRequest(m_station->GetStationAddress());
                }
                const common::TimeSpan retryInterval = common::TimeSpan::FromMSec(m_retryIntervalMsec);
                m_retryDeadline = common::g_piaJobScheduler->m_dispatchTime + retryInterval;
            }
            return MakeExecuteResult(cExecuteKindNextStep);
        }
    }
    m_station->m_field20 = 6;
    SelectStep(m_currentStep, &DisconnectStationJob::DisconnectionSucceeded,
               "DisconnectStationJob::DisconnectionSucceeded");
    return MakeExecuteResult(cExecuteKindContinue);
}

DisconnectStationJob::ExecuteResult DisconnectStationJob::DisconnectionSucceeded()
{
    m_station->m_disconnectStationJob->ProcessDisconnection(m_station);
    g_stationConnectionInfoTable->EraseFromTable(m_station);
    IdentificationInfoTable::GetInstance()->EraseFromTable(m_station);
    ConnectStationJob* connectJob = m_station->m_connectStationJob;
    if (connectJob)
    {
        connectJob->Cleanup();
        connectJob->Reset(false);
    }
    ProcessConnectionRequestJob* processJob = m_station->m_processConnectionRequestJob;
    if (processJob)
    {
        processJob->Cleanup();
        processJob->Reset(false);
    }
    m_station->Cleanup();
    StationManager::s_pInstance->DestroyStation(m_station);
    return MakeExecuteResult(cExecuteKindFinished);
}

DisconnectStationJob::ExecuteResult DisconnectStationJob::CutRouteOfRelayConnection()
{
    if (m_station->GetStationIndex() <= StationManager::cStationNumMax - 1)
    {
        m_station->m_field28->SendDisconnectionRequest(m_station->GetStationIndex(), true);
        m_station->m_field28->SendDisconnectionRequest(m_station->GetStationIndex(), true);
    }
    m_station->m_field20 = 6;
    SelectStep(m_currentStep, &DisconnectStationJob::DisconnectionSucceeded,
               "DisconnectStationJob::DisconnectionSucceeded");
    return MakeExecuteResult(cExecuteKindNextStep);
}

void DisconnectStationJob::Cleanup()
{
    m_station = 0;
    m_waiting = false;
    m_deadline = common::Time();
}

void DisconnectStationJob::Trace(unsigned long long) const
{
}

void DisconnectStationJob::ProcessDisconnection(Station*)
{
}

Result DisconnectStationJob::StartupImpl(Station* station)
{
    if (!station)
        return ResultInvalidArgument();
    if (station->m_field20 != 5)
        return ResultInvalidState();
    common::Time now;
    m_station = station;
    m_waiting = false;
    now.SetNow();
    m_deadline = now + common::TimeSpan::FromMSec(m_timeoutMsec);
    Reset(true);
    SelectStep(m_currentStep, &DisconnectStationJob::SendDisconnectionRequest,
               "DisconnectStationJob::SendDisconnectionRequest");
    if (g_piaTransport->m_relayRouteManager && StationManager::s_pInstance->m_localStation)
    {
        StationIndex relay;
        Result result = g_piaTransport->m_relayRouteManager->GetRelayRoute(
            StationManager::s_pInstance->m_localStation->GetStationIndex(), m_station->GetStationIndex(), &relay);
        if (result == 0 && relay != m_station->GetStationIndex())
        {
            SelectStep(m_currentStep, &DisconnectStationJob::CutRouteOfRelayConnection,
                       "DisconnectStationJob::CutRouteOfRelayConnection");
        }
    }
    return 0;
}

}
}
}
