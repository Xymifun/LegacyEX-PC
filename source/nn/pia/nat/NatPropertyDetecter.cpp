#include "nn/pia/nat/NatPropertyDetecter.hpp"

#include "nn/pia/common/TimeSpan.hpp"

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
    cResultInvalidState = 0x10c08,
    cTraceReplyArray = 0x1000,
    cProbeRequestFirst = 0x65,
    cProbeRequestSecond = 0x66,
    cProbeRequestThird = 0x67,
    cProbeRepeatCount = 5,
    cMaxRetryCount = 3,
    cDetectionTimeoutMsec = 2000,
    cMappingTypeSame = 1,
    cMappingTypeDifferent = 2,
    cFilteringTypeAnswered = 1,
    cFilteringTypeSilent = 2,
    cPortDeltaStep = 1
};
}

unsigned short NatPropertyDetecter::s_lastDetectionMilliseconds;

NatPropertyDetecter::NatPropertyDetecter() : NatDetecter(), m_startTime()
{
}

NatPropertyDetecter::~NatPropertyDetecter()
{
}

Result NatPropertyDetecter::Startup(NatTraverser* traverser, common::CallContext* context,
                                    const common::InetAddress& address)
{
    common::Time now;
    now.SetNow();
    m_startTime = now;
    if (m_detectionJob.IsRunning())
        return cResultInvalidState;

    Result result = NatDetecter::Startup(traverser, context, address);
    if (result != 0)
        return result;
    result = m_detectionJob.Startup(context, this, traverser);
    if (result != 0)
        return result;
    return 0;
}

void NatPropertyDetecter::Cleanup()
{
    common::Time now;
    now.SetNow();
    s_lastDetectionMilliseconds = static_cast<unsigned short>((now - m_startTime).m_tick
                                                              / common::TimeSpan::GetTicksPerMSec().m_tick);
    NatDetecter::Cleanup();
}

void NatPropertyDetecter::SendNatPropertyDetectionProbes()
{
    common::InetAddress* primary = GetPrimaryServerPrimaryPortAddress();

    AddNatCheckProbe(NatCheckProbe(*primary, cProbeRequestFirst), cProbeRepeatCount);
    AddNatCheckProbe(NatCheckProbe(*primary, cProbeRequestSecond), cProbeRepeatCount);

    common::SimpleContainer<common::InetAddress, 4>::iterator it = m_traverser->m_servers.begin();
    common::SimpleContainer<common::InetAddress, 4>::iterator end = m_traverser->m_servers.end();
    if (it == end)
        return;
    for (++it; it != end; ++it)
    {
        if (it->m_address != primary->m_address && it->m_port == primary->m_port)
        {
            AddNatCheckProbe(NatCheckProbe(*it, cProbeRequestThird), cProbeRepeatCount);
            return;
        }
    }
}

void NatPropertyDetecter::StartSendingNatCheckProbe()
{
    SendDummyProbe();
    SendNatPropertyDetectionProbes();
}

bool NatPropertyDetecter::IsReceivedAllMessage()
{
    if (m_replies[0].count != 0 && m_replies[1].count != 0 && m_replies[2].count != 0)
        return true;
    return false;
}

unsigned int NatPropertyDetecter::HandleResult()
{
    TraceNatCheckReplyArray(cTraceReplyArray);
    if (m_replies[0].body.portWord == 0 || m_replies[2].body.portWord == 0)
        return 0;

    NatTraverser::NatProperty& property = m_traverser->m_natProperty;
    property.m_privatePort = m_primaryAddress.m_port;
    property.m_publicPort = static_cast<unsigned short>(m_replies[2].body.portWord);
    property.m_filteringType = m_replies[1].body.portWord == 0 ? cFilteringTypeSilent : cFilteringTypeAnswered;
    if (m_replies[0].body.portWord == m_replies[2].body.portWord)
    {
        property.m_mappingType = cMappingTypeSame;
    }
    else
    {
        property.m_mappingType = cMappingTypeDifferent;
        property.m_portDelta = cPortDeltaStep;
        if (static_cast<unsigned short>(m_replies[2].body.portWord) == m_primaryAddress.m_port)
        {
            property.m_portDelta = cPortDeltaStep;
            property.m_flag = 1;
        }
    }
    return 1;
}

unsigned int NatPropertyDetecter::CheckRetry()
{
    if ((GetNatCheckReply(0)->body.portWord == 0 || GetNatCheckReply(2)->body.portWord == 0)
        && m_retryCount < cMaxRetryCount)
        return 1;
    return 0;
}

unsigned int NatPropertyDetecter::GetDetectionTimeout() const
{
    return cDetectionTimeoutMsec;
}

}
}
}
