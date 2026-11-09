#include "nn/pia/inet/NexNatRelayInterface.hpp"

#include "nn/nex/NATTraversalRelayClient.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/RelaySignatureKey.hpp"
#include "nn/pia/inet/NatRelayInterface.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/transport/StationLocation.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{

enum
{
    cTraceNatRelay = 0x800
};

}

void NexNatRelayInterface::RegisterRelayClient(nex::NATTraversalRelayClient* relayClient)
{
    if (relayClient != 0)
        m_relayClient = relayClient;
}

void NexNatRelayInterface::UnregisterRelayClient()
{
    m_relayClient = 0;
}

void NexNatRelayInterface::RelayProbeRequest(const nex::qList<nex::StationURL>& targetUrls,
                                             const nex::StationURL& sourceUrl)
{
    if (m_callContext->GetState() == nex::CallContext::CallInProgress)
        m_callContext->Cancel(nex::CallContext::CallCanceled);
    m_callContext->Reset();
    m_relayClient->CallRequestProbeInitiationExt(m_callContext, targetUrls, sourceUrl);
}

void NexNatRelayInterface::RequestProbe(const nex::StationURL& targetUrl)
{
    if (m_natRelay != 0)
    {
        transport::StationLocation location;
        NexFacade::ConvertNexStationUrlToStationLocation(targetUrl, &location);
        location.Trace(cTraceNatRelay);
        m_natRelay->RequestProbe(location);
    }
}

void NexNatRelayInterface::ReportNATTraversalResult(const unsigned int& connectionId, const bool& succeeded,
                                                    const unsigned int& rtt)
{
    if (m_callContext->GetState() == nex::CallContext::CallInProgress)
        m_callContext->Cancel(nex::CallContext::CallCanceled);
    m_callContext->Reset();
    m_relayClient->CallReportNATTraversalResult(m_callContext, connectionId, succeeded, rtt);
}

void NexNatRelayInterface::ReportNATProperties(unsigned int mapping, unsigned int filtering, unsigned int portIncrement)
{
    if (m_callContext->GetState() == nex::CallContext::CallInProgress)
        m_callContext->Cancel(nex::CallContext::CallCanceled);
    m_callContext->Reset();
    m_relayClient->CallReportNATProperties(m_callContext, mapping, filtering, portIncrement);
}

void NexNatRelayInterface::UpdateConnectionState(unsigned int connectionId, const nex::StationURL&)
{
    if (m_natRelay != 0)
        m_natRelay->SetLocalCID(connectionId);
}

bool NexNatRelayInterface::GetRelaySignatureKey(nex::CallContext* callContext, nex::RelaySignatureKey* key)
{
    if (NexFacade::GetInstance()->IsRelayServiceEnabled())
    {
        bool result = false;
        if (key != 0 && callContext != 0)
        {
            if (callContext->GetState() == nex::CallContext::CallInProgress)
                callContext->Cancel(nex::CallContext::CallCanceled);
            callContext->Reset();
            result = m_relayClient->CallGetRelaySignatureKey(
                static_cast<nex::ProtocolCallContext*>(callContext), &key->m_relayMode, &key->m_currentUTCTime,
                &key->m_relayAddress, &key->m_relayPort, &key->m_relayAddressType, &key->m_gameServerID);
        }
        return result;
    }
    else
    {
        return false;
    }
}

}
}
}
