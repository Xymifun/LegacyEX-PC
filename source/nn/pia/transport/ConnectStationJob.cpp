#include "nn/pia/transport/ConnectStationJob.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/transport/ResendingMessageManager.hpp"
#include "nn/pia/transport/StationProtocol.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/Transport.hpp"

#include <cstring>

namespace nn
{
namespace pia
{
namespace transport
{
namespace
{
typedef common::Job::ExecuteResult (ConnectStationJob::*Step)();

inline void SelectStep(common::StepSequenceJob::Step& slot, Step function, const char* name)
{
    const common::StepSequenceJob::Step step(
        static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

ConnectStationJob::ConnectStationJob()
    : m_connectionDeadline(),
      m_timeoutMsec(10000),
      m_requestAckId(0),
      m_context(0),
      m_station(0),
      m_connectionRejection(0)
{
}

ConnectStationJob::~ConnectStationJob()
{
}

Result ConnectStationJob::Startup(common::CallContext* context, Station* station, const StationConnectionInfo& info,
                                  bool secure, int timeoutMsec)
{
    m_timeoutMsec = timeoutMsec;
    return StartupImpl(context, station, info, secure);
}

Result ConnectStationJob::StartupRelayConnection(common::CallContext* context, Station* station,
                                                 const StationConnectionInfo& info, bool secure, int timeoutMsec)
{
    m_timeoutMsec = timeoutMsec;
    const Result result = StartupImpl(context, station, info, secure);
    if (result != 0)
        return result;
    m_station->m_field56 = 0;
    SetCurrentStep(
        Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&ConnectStationJob::SendRelayConnectionRequest),
             "ConnectStationJob::SendRelayConnectionRequest"));
    return 0;
}

Result ConnectStationJob::StartupImpl(common::CallContext* context, Station* station, const StationConnectionInfo& info,
                                      bool secure)
{
    if (!station)
        return ResultInvalidArgument();
    station->Trace(0x08000000);
    if (m_station)
        m_station->Trace(0x08000000);
    if (GetState() != cStateNotEntried && GetState() != cStateCompleted)
    {
        Trace(0x8000);
        return ResultInvalidState();
    }
    if (!context)
        return ResultInvalidArgument();
    m_context = context;
    context->InitiateCall();
    m_station = station;
    station->m_stationAddress = info.m_stationLocation.m_stationAddress;
    m_secure = secure;
    Reset(true);
    m_station->m_field56 = 1;
    SetCurrentStep(
        Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&ConnectStationJob::SendConnectionRequest),
             "ConnectStationJob::SendConnectionRequest"));
    return 0;
}

void ConnectStationJob::Cleanup()
{
    CleanupImpl();
    m_station = 0;
    if (m_requestAckId)
    {
        if (g_piaResendingMessageManager)
            g_piaResendingMessageManager->StopResending(m_requestAckId);
        m_requestAckId = 0;
    }
    m_connectionDeadline.m_tick = 0;
    if (m_context)
    {
        if (static_cast<unsigned int>(m_context->m_state) - 2 > 2)
            m_context->SignalCancel();
        m_context = 0;
    }
    if (IsRunning())
        Reset(false);
}

void ConnectStationJob::Trace(unsigned long long) const
{
}

void ConnectStationJob::CleanupImpl()
{
}

bool ConnectStationJob::CheckErrorOccurredImpl()
{
    return false;
}

bool ConnectStationJob::CheckErrorOccurred()
{
    return CheckErrorOccurredImpl();
}

bool ConnectStationJob::CheckContextCallCancelled()
{
    if (m_context && m_context->m_cancelRequested)
    {
        if (static_cast<unsigned int>(m_context->m_state) - 2 > 2)
            m_context->SignalCancel();
        m_context = 0;
        return true;
    }
    return false;
}

bool ConnectStationJob::CheckTimeout()
{
    if (m_connectionDeadline <= common::g_piaJobScheduler->m_dispatchTime)
    {
        if (m_context)
        {
            if (static_cast<unsigned int>(m_context->m_state) - 2 > 2)
                m_context->SignalFailure(cResultTimeout);
            m_context = 0;
        }
        return true;
    }
    return false;
}

ConnectStationJob::ExecuteResult ConnectStationJob::SendConnectionRequest()
{
    if (CheckContextCallCancelled())
    {
        if (m_station && m_station->m_field20 == 4)
            m_station->m_field20 = 6;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (!m_station->m_field54)
    {
        common::Time now;
        now.SetNow();
        const unsigned char connectionId = static_cast<unsigned char>(now.m_tick % 0xfe + 2);
        m_station->m_field54 = connectionId;
    }
    if (!CheckErrorOccurred() && SendConnectionRequestMessage())
    {
        if (m_station->GetStationIndex() > 31)
        {
            m_station->m_lastSendTime = g_piaTransport->m_time;
        }
        m_connectionRejection = 0;
        const common::TimeSpan timeout = common::TimeSpan::FromMSec(m_timeoutMsec);
        m_connectionDeadline = common::g_piaJobScheduler->m_dispatchTime + timeout;
        SelectStep(m_currentStep, &ConnectStationJob::WaitRequestAck, "ConnectStationJob::WaitRequestAck");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    SelectStep(m_currentStep, &ConnectStationJob::ConnectionFailed, "ConnectStationJob::ConnectionFailed");
    return MakeExecuteResult(cExecuteKindContinue);
}

ConnectStationJob::ExecuteResult ConnectStationJob::WaitRequestAck()
{
    if (g_piaResendingMessageManager->CheckNowResending(m_requestAckId))
    {
        if (CheckContextCallCancelled())
        {
            g_piaResendingMessageManager->StopResending(m_requestAckId);
            m_requestAckId = 0;
            if (m_station && m_station->m_field20 == 4)
                m_station->m_field20 = 6;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (CheckTimeout() || CheckErrorOccurred())
        {
            g_piaResendingMessageManager->StopResending(m_requestAckId);
            m_requestAckId = 0;
            SelectStep(m_currentStep, &ConnectStationJob::ConnectionFailed, "ConnectStationJob::ConnectionFailed");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (m_station->m_field20 == 5)
        {
            g_piaResendingMessageManager->StopResending(m_requestAckId);
            m_requestAckId = 0;
            SelectStep(m_currentStep, &ConnectStationJob::ConnectionSucceeded,
                       "ConnectStationJob::ConnectionSucceeded");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (m_connectionRejection)
        {
            if (m_context)
            {
                if (static_cast<unsigned int>(m_context->m_state) - 2 > 2)
                {
                    switch (m_connectionRejection)
                    {
                    case 1:
                        m_context->SignalFailure(cResultConnectionRefused);
                        break;
                    case 2:
                        m_context->SignalFailure(cResultConnectionFailed10c25);
                        break;
                    default:
                        m_context->SignalFailure(cResultConnectionFailed);
                        break;
                    }
                }
                m_context = 0;
            }
            g_piaResendingMessageManager->StopResending(m_requestAckId);
            m_requestAckId = 0;
            if (m_station && m_station->m_field20 == 4)
                m_station->m_field20 = 6;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    SelectStep(m_currentStep, &ConnectStationJob::WaitForConnection, "ConnectStationJob::WaitForConnection");
    m_requestAckId = 0;
    return MakeExecuteResult(cExecuteKindContinue);
}

ConnectStationJob::ExecuteResult ConnectStationJob::WaitForConnection()
{
    if (m_station->m_field20 != 5)
    {
        if (CheckContextCallCancelled())
        {
            if (m_station && m_station->m_field20 == 4)
                m_station->m_field20 = 6;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (CheckTimeout() || CheckErrorOccurred())
        {
            SelectStep(m_currentStep, &ConnectStationJob::ConnectionFailed, "ConnectStationJob::ConnectionFailed");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    SelectStep(m_currentStep, &ConnectStationJob::ConnectionSucceeded, "ConnectStationJob::ConnectionSucceeded");
    return MakeExecuteResult(cExecuteKindContinue);
}

ConnectStationJob::ExecuteResult ConnectStationJob::ConnectionFailed()
{
    if (m_station && m_station->m_field20 == 4)
        m_station->m_field20 = 6;
    if (m_context && static_cast<unsigned int>(m_context->m_state) - 2 > 2)
        m_context->SignalFailure(cResultConnectionFailed);
    m_context = 0;
    return MakeExecuteResult(cExecuteKindFinished);
}

ConnectStationJob::ExecuteResult ConnectStationJob::ConnectionSucceeded()
{
    if (CheckContextCallCancelled())
        return MakeExecuteResult(cExecuteKindFinished);
    if (CheckErrorOccurred())
    {
        SelectStep(m_currentStep, &ConnectStationJob::ConnectionFailed, "ConnectStationJob::ConnectionFailed");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_station->GetStationIndex() <= 31)
        g_piaTransport->OutputStreamUpdateEvent();
    if (m_context && static_cast<unsigned int>(m_context->m_state) - 2 > 2)
        m_context->SignalSuccess(0);
    m_context = 0;
    return MakeExecuteResult(cExecuteKindFinished);
}

ConnectStationJob::ExecuteResult ConnectStationJob::SendRelayConnectionRequest()
{
    if (CheckContextCallCancelled())
    {
        if (m_station && m_station->m_field20 == 4)
            m_station->m_field20 = 6;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (SendRelayConnectionRequestMessage())
    {
        m_connectionRejection = 0;
        const common::TimeSpan timeout = common::TimeSpan::FromMSec(m_timeoutMsec);
        m_connectionDeadline = common::g_piaJobScheduler->m_dispatchTime + timeout;
        SelectStep(m_currentStep, &ConnectStationJob::WaitRequestAck, "ConnectStationJob::WaitRequestAck");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

bool ConnectStationJob::SendConnectionRequestMessage()
{
    if (!m_station->m_field0)
        return false;
    StationProtocol* protocol = m_station->m_field28;

    unsigned long long principalId;
    Result result = m_station->GetPrincipalId(&principalId);
    if (result != 0)
        return false;

    StationConnectionInfo info;
    result = g_stationConnectionInfoTable->GetStationConnectionInfo(m_station, &info);
    if (result != 0)
        return false;

    unsigned char buffer[128] = {};
    protocol->MakeConnectionRequestData(buffer, m_station->m_field54, principalId,
                                        static_cast<unsigned int>(info.m_stationLocation.m_field18),
                                        m_station->m_field55, false, m_secure);

    common::StationAddress stationAddress;
    if (g_piaTransport->m_relayRouteManager == 0 && m_station->GetStationIndex() != cStationIndexReserved)
    {
        if (m_station->GetStationIndex() <= 31)
        {
            const StationIndex stationIndex = m_station->GetStationIndex();
            if (stationIndex != pia::cStationIndexInvalid)
            {
                return g_piaResendingMessageManager->SetSendMessageByStationIndex(
                           &m_requestAckId, buffer, protocol->GetConnectionRequestDataSize(), stationIndex,
                           protocol->mProtocolId, false, 0)
                       == 0;
            }
        }
        else
        {
            stationAddress = m_station->m_stationAddress;
        }
    }
    else
    {
        stationAddress = m_station->m_stationAddress;
    }
    return g_piaResendingMessageManager->SetSendMessageByStationAddress(&m_requestAckId, buffer,
                                                                        protocol->GetConnectionRequestDataSize(),
                                                                        stationAddress, protocol->mProtocolId, 0)
           == 0;
}

bool ConnectStationJob::SendRelayConnectionRequestMessage()
{
    if (!m_station->m_field0 || m_station->GetStationIndex() > 31)
        return false;
    StationProtocol* protocol = m_station->m_field28;

    unsigned long long principalId;
    Result result = m_station->GetPrincipalId(&principalId);
    if (result != 0)
        return false;

    StationConnectionInfo info;
    result = g_stationConnectionInfoTable->GetStationConnectionInfo(m_station, &info);
    if (result != 0)
        return false;

    unsigned char buffer[128] = {};
    protocol->MakeConnectionRequestData(buffer, m_station->m_field54, principalId,
                                        static_cast<unsigned int>(info.m_stationLocation.m_field18),
                                        m_station->m_field55, true, m_secure);
    result = g_piaResendingMessageManager->SetSendMessageByStationIndex(
        &m_requestAckId, buffer, protocol->GetConnectionRequestDataSize(), m_station->GetStationIndex(),
        protocol->mProtocolId, false, 0);
    return result == 0;
}

}
}
}
