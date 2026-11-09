#include "nn/pia/nat/NatPortDetecter.hpp"

#include "nn/pia/nat/Api.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

namespace
{
enum
{
    cProbeRepeatCount = 5,
    cMaxRetryCount = 3,
    cMappingTypeDifferent = 2
};
}

NatPortDetecter::NatPortDetecter() : NatDetecter(), m_primaryStation(), m_secondaryStation(), m_startTime()
{
}

NatPortDetecter::~NatPortDetecter()
{
}

Result NatPortDetecter::Startup(NatTraverser* traverser, common::CallContext* context,
                                const common::InetAddress& address)
{
    m_startTime.SetNow();
    return NatDetecter::Startup(traverser, context, address);
}

Result NatPortDetecter::Startup(NatTraverser* traverser, common::CallContext* context,
                                const common::InetAddress& address, const transport::StationLocation& primary,
                                const transport::StationLocation& secondary, bool relay)
{
    if (m_detectionJob.IsRunning())
        return ResultInvalidState();

    m_primaryStation = primary;
    m_secondaryStation = secondary;
    m_relay = relay;
    Result result = Startup(traverser, context, address);
    if (result != 0)
        return result;
    result = m_detectionJob.Startup(context, this, traverser);
    if (result != 0)
        return result;
    return 0;
}

void NatPortDetecter::Cleanup()
{
    NatDetecter::Cleanup();
}

void NatPortDetecter::SendNatPortDetectionProbes()
{
    common::InetAddress* secondary = GetPrimaryServerSecondaryPortAddress();
    if (secondary != 0 && m_traverser->m_natProperty.m_mappingType == cMappingTypeDifferent)
        AddNatCheckProbe(NatCheckProbe(*secondary, cPortDetectionProbeRequest), cProbeRepeatCount);
    else
        AddNatCheckProbe(NatCheckProbe(*GetPrimaryServerPrimaryPortAddress(), cPortDetectionProbeRequest),
                         cProbeRepeatCount);
}

void NatPortDetecter::StartSendingNatCheckProbe()
{
    SendDummyProbe();
    SendNatPortDetectionProbes();
}

bool NatPortDetecter::IsReceivedAllMessage()
{
    return m_replies[0].count != 0;
}

unsigned int NatPortDetecter::HandleResult()
{
    TraceNatCheckReplyArray(cTraceDetail);
    PredictPort(&m_secondaryStation.m_stationAddress.m_inetAddress);
    if (!m_secondaryStation.m_stationAddress.m_inetAddress.IsValid())
        return 0;

    common::Time now;
    now.SetNow();
    const unsigned short elapsedMsec = static_cast<unsigned short>(
        static_cast<long long>(now.m_tick - m_startTime.m_tick) / common::TimeSpan::GetTicksPerMSec().m_tick);
    NatTraverser* traverser = m_traverser;
    if (m_relay)
    {
        traverser->RelayProbeRequest(m_primaryStation, m_secondaryStation, m_relay, elapsedMsec);
    }
    else
    {
        traverser->m_natProperty.m_privatePort = m_primaryAddress.m_port;
        traverser->m_natProperty.m_publicPort = GetPerceivedPort();
        common::g_sessionBeginMonitoringContent.m_metric154 = elapsedMsec;
    }
    return 1;
}

unsigned int NatPortDetecter::CheckRetry()
{
    const unsigned short perceivedPort = GetPerceivedPort();
    if (perceivedPort == 0 && m_retryCount < cMaxRetryCount)
        return 1;

    if (perceivedPort < NatDetecter::GetCurrentPort() && m_relay && m_retryCount < cMaxRetryCount)
    {
        if (RetryBind() != 0)
            return 0;
        return 1;
    }

    NatDetecter::SetCurrentPort(perceivedPort);
    return 0;
}

unsigned int NatPortDetecter::GetDetectionTimeout() const
{
    return 2000;
}

}
}
}
