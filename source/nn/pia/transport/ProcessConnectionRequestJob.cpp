#include "nn/pia/transport/ProcessConnectionRequestJob.hpp"

#include "nn/pia/PayloadSizeManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/transport/ConnectStationJob.hpp"
#include "nn/pia/transport/RelayRouteManager.hpp"
#include "nn/pia/transport/ResendingMessageManager.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"
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
typedef common::Job::ExecuteResult (ProcessConnectionRequestJob::*Step)();

enum
{
    cTraceSession = 0x08000000
};

inline void SelectStep(common::StepSequenceJob::Step& slot, Step function, const char* name)
{
    const common::StepSequenceJob::Step step(
        static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

ProcessConnectionRequestJob::ProcessConnectionRequestJob() : m_deadline(), m_timeoutMsec(10000)
{
    m_callContext = common::NewObj<common::CallContext>();
    m_responseAckIds[0] = 0;
    m_responseAckIds[1] = 0;
}

ProcessConnectionRequestJob::~ProcessConnectionRequestJob()
{
    if (m_callContext != NULL)
    {
        if (m_callContext->m_state == common::CallContext::cStateInProgress)
            m_callContext->SignalCancel();
        common::DeleteObj(m_callContext);
        m_callContext = NULL;
    }
}

bool ProcessConnectionRequestJob::Startup(Station* station, int timeoutMsec, bool secure)
{
    if (station == NULL)
        return false;
    m_station = station;
    m_station->Trace(cTraceSession);
    m_callContext->SignalSuccess(0);
    if (m_station->m_field20 == 3 && !secure)
    {
        ConnectStationJob* connectJob = m_station->m_connectStationJob;
        StationConnectionInfo info;
        if (g_stationConnectionInfoTable->GetStationConnectionInfo(m_station, &info) == 0
            && connectJob->Startup(m_callContext, m_station, info, true, timeoutMsec) == 0)
            connectJob->Ready(false);
    }
    else if (m_station->m_field20 == 3 || !secure)
    {
        return false;
    }
    m_timeoutMsec = timeoutMsec;
    m_station->m_field20 = 4;
    const common::TimeSpan timeout = common::TimeSpan::FromMSec(m_timeoutMsec);
    m_deadline = common::g_piaJobScheduler->m_dispatchTime + timeout;
    m_isRelayConnection = false;
    m_responseAckIds[0] = 0;
    m_responseAckIds[1] = 0;
    Reset(true);
    SelectStep(m_currentStep, &ProcessConnectionRequestJob::ConnectToRequesterStation,
               "ProcessConnectionRequestJob::ConnectToRequesterStation");
    return true;
}

bool ProcessConnectionRequestJob::StartupRelayConnection(Station* station, int timeoutMsec, bool secure)
{
    if (station == NULL)
        return false;
    m_station = station;
    m_station->Trace(cTraceSession);
    m_callContext->SignalSuccess(0);
    if (m_station->m_field20 == 3 && !secure)
    {
        ConnectStationJob* connectJob = m_station->m_connectStationJob;
        StationConnectionInfo info;
        if (g_stationConnectionInfoTable->GetStationConnectionInfo(m_station, &info) == 0
            && connectJob->StartupRelayConnection(m_callContext, m_station, info, true, timeoutMsec) == 0)
            connectJob->Ready(false);
    }
    else if (m_station->m_field20 == 3 || !secure)
    {
        return false;
    }
    m_timeoutMsec = timeoutMsec;
    m_station->m_field20 = 4;
    const common::TimeSpan timeout = common::TimeSpan::FromMSec(m_timeoutMsec);
    m_deadline = common::g_piaJobScheduler->m_dispatchTime + timeout;
    m_responseAckIds[0] = 0;
    m_responseAckIds[1] = 0;
    m_isRelayConnection = true;
    Reset(true);
    SelectStep(m_currentStep, &ProcessConnectionRequestJob::WaitInverseConnection,
               "ProcessConnectionRequestJob::WaitInverseConnection");
    return true;
}

ProcessConnectionRequestJob::ExecuteResult ProcessConnectionRequestJob::ConnectToRequesterStation()
{
    SelectStep(m_currentStep, &ProcessConnectionRequestJob::WaitInverseConnection,
               "ProcessConnectionRequestJob::WaitInverseConnection");
    return MakeExecuteResult(cExecuteKindContinue);
}

ProcessConnectionRequestJob::ExecuteResult ProcessConnectionRequestJob::WaitInverseConnection()
{
    switch (m_callContext->m_state)
    {
    case common::CallContext::cStateNotCalled:
        break;
    case common::CallContext::cStateInProgress:
        return MakeExecuteResult(cExecuteKindNextStep);
    case common::CallContext::cStateSuccess:
    case common::CallContext::cStateFailure:
    case common::CallContext::cStateCancelled:
        if (m_callContext->m_result != 0)
            return MakeExecuteResult(cExecuteKindFinished);
        break;
    default:
        break;
    }
    const int remainingMsec = static_cast<int>(
        (m_station->m_connectStationJob->m_connectionDeadline - m_deadline).m_tick
        / common::TimeSpan::GetTicksPerMSec().m_tick);
    if (remainingMsec > 0)
        m_deadline = m_station->m_connectStationJob->m_connectionDeadline;
    SelectStep(m_currentStep, &ProcessConnectionRequestJob::SendConnectionResponse,
               "ProcessConnectionRequestJob::SendConnectionResponse");
    return MakeExecuteResult(cExecuteKindContinue);
}

bool ProcessConnectionRequestJob::TimeOutCheck()
{
    if (m_deadline <= common::g_piaJobScheduler->m_dispatchTime)
    {
        m_callContext->Cancel();
        return true;
    }
    return false;
}

ProcessConnectionRequestJob::ExecuteResult ProcessConnectionRequestJob::SendConnectionResponse()
{
    if (!m_station->m_field0)
        return MakeExecuteResult(cExecuteKindFinished);

    unsigned long long principalId;
    if (m_station->GetPrincipalId(&principalId) != 0)
        return MakeExecuteResult(cExecuteKindFinished);

    StationConnectionInfo info;
    if (g_stationConnectionInfoTable->GetStationConnectionInfo(m_station, &info) != 0)
        return MakeExecuteResult(cExecuteKindFinished);

    if (common::g_payloadSizeManager->m_mtuSize
            - (common::g_payloadSizeManager->m_signatureSize + cResponseOverheadSize)
        < cResponseDataSize)
    {
        bool failed = false;
        for (int i = 0; i < cResponseMessageNum; ++i)
        {
            unsigned char buffer[cDividedResponseDataSize] = {};
            m_station->m_field28->MakeConnectionResponseData(
                buffer, static_cast<unsigned char>(i + 1), principalId,
                static_cast<unsigned int>(info.m_stationLocation.m_field18), m_isRelayConnection);
            m_responseAckIds[i] = 0;
            const StationProtocol* protocol = m_station->m_field28;
            if (m_isRelayConnection)
            {
                if (g_piaResendingMessageManager->SetSendMessageByStationIndex(
                        &m_responseAckIds[i], buffer, cDividedResponseDataSize, m_station->GetStationIndex(),
                        protocol->GetProtocolId(), true, m_deadline.m_tick)
                    != 0)
                    failed = true;
            }
            else
            {
                if (g_piaResendingMessageManager->SetSendMessageByStationAddress(
                        &m_responseAckIds[i], buffer, cDividedResponseDataSize, m_station->m_stationAddress,
                        protocol->GetProtocolId(), m_deadline.m_tick)
                    != 0)
                    failed = true;
            }
        }
        if (failed)
        {
            for (int i = 0; i < cResponseMessageNum; ++i)
            {
                if (m_responseAckIds[i] != 0)
                    g_piaResendingMessageManager->StopResending(m_responseAckIds[i]);
            }
            if (TimeOutCheck())
                return MakeExecuteResult(cExecuteKindFinished);
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (m_station->GetStationIndex() > 31)
            m_station->m_lastSendTime = g_piaTransport->m_time;
        SelectStep(m_currentStep, &ProcessConnectionRequestJob::WaitResponseAck,
                   "ProcessConnectionRequestJob::WaitResponseAck");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    else
    {
        unsigned char buffer[cResponseDataSize] = {};
        m_station->m_field28->MakeConnectionResponseData(
            buffer, 0, principalId, static_cast<unsigned int>(info.m_stationLocation.m_field18), m_isRelayConnection);
        const StationProtocol* protocol = m_station->m_field28;
        Result result;
        if (m_isRelayConnection)
        {
            result = g_piaResendingMessageManager->SetSendMessageByStationIndex(
                &m_responseAckIds[0], buffer, cResponseDataSize, m_station->GetStationIndex(),
                protocol->GetProtocolId(), true, m_deadline.m_tick);
        }
        else
        {
            result = g_piaResendingMessageManager->SetSendMessageByStationAddress(
                &m_responseAckIds[0], buffer, cResponseDataSize, m_station->m_stationAddress, protocol->GetProtocolId(),
                m_deadline.m_tick);
        }
        if (result != 0)
        {
            if (TimeOutCheck())
                return MakeExecuteResult(cExecuteKindFinished);
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (m_station->GetStationIndex() > 31)
            m_station->m_lastSendTime = g_piaTransport->m_time;
        SelectStep(m_currentStep, &ProcessConnectionRequestJob::WaitResponseAck,
                   "ProcessConnectionRequestJob::WaitResponseAck");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
}

ProcessConnectionRequestJob::ExecuteResult ProcessConnectionRequestJob::WaitResponseAck()
{
    bool acknowledged = true;
    for (int i = 0; i < cResponseMessageNum; ++i)
    {
        if (g_piaResendingMessageManager->CheckNowResending(m_responseAckIds[i]))
            acknowledged = false;
    }
    if (!acknowledged)
    {
        if (TimeOutCheck())
        {
            for (int i = 0; i < cResponseMessageNum; ++i)
            {
                g_piaResendingMessageManager->StopResending(m_responseAckIds[i]);
                m_responseAckIds[i] = 0;
            }
            return MakeExecuteResult(cExecuteKindFinished);
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (!m_isRelayConnection && g_piaTransport->m_relayRouteManager != NULL && m_station->GetStationIndex() <= 31)
    {
        if (StationManager::s_pInstance->m_localStation->GetStationIndex() <= 31)
        {
            g_piaTransport->m_relayRouteManager->SetRelayRoute(
                StationManager::s_pInstance->m_localStation->GetStationIndex(), m_station->GetStationIndex(),
                m_station->GetStationIndex());
        }
    }
    m_responseAckIds[0] = 0;
    m_responseAckIds[1] = 0;
    return MakeExecuteResult(cExecuteKindFinished);
}

void ProcessConnectionRequestJob::Cleanup()
{
    m_station = NULL;
    for (int i = 0; i < cResponseMessageNum; ++i)
    {
        if (m_responseAckIds[i] != 0)
        {
            if (g_piaResendingMessageManager != NULL)
                g_piaResendingMessageManager->StopResending(m_responseAckIds[i]);
            m_responseAckIds[i] = 0;
        }
    }
    m_deadline.m_tick = 0;
    if (m_callContext->m_state == common::CallContext::cStateInProgress)
        m_callContext->SignalCancel();
    if (IsRunning())
        Reset(false);
}

void ProcessConnectionRequestJob::Trace(unsigned long long) const
{
}

}
}
}
