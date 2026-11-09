#include "nn/nex/RVNATRelay.hpp"

#include "nn/nex/Job.hpp"
#include "nn/nex/JobProtocolCallContextCallback.hpp"
#include "nn/nex/NATCheckClient.hpp"
#include "nn/nex/NATTraversalEngine.hpp"
#include "nn/nex/NATTraversalRelayClient.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/RelaySignatureKey.hpp"

namespace nn
{
namespace nex
{

RVNATRelay::RVNATRelay()
{
    m_pRelayClient = 0;
    m_pEngine = 0;
}

RVNATRelay::~RVNATRelay()
{
    m_pRelayClient = 0;
    if (m_pEngine != 0)
    {
        m_pEngine->UnregisterRelay();
        m_pEngine->UnregisterEcho();
        m_pEngine->UnregisterConnectivityTester();
        m_pEngine = 0;
    }
}

void RVNATRelay::RegisterRelayClient(NATTraversalRelayClient* pRelayClient)
{
    m_pRelayClient = pRelayClient;
}

void RVNATRelay::UnregisterRelayClient()
{
    m_pRelayClient = 0;
}

void RVNATRelay::RelayProbeRequest(const qList<StationURL>& rTargetURLs)
{
    ProtocolCallContext callContext(0);
    m_pRelayClient->CallRequestProbeInitiation(&callContext, rTargetURLs);
}

void RVNATRelay::RelayProbeRequest(const qList<StationURL>& rTargetURLs, const StationURL& rSourceURL)
{
    qList<StationURL>::const_iterator last = rTargetURLs.end();
    for (qList<StationURL>::const_iterator url = rTargetURLs.begin(); url != last; url++)
    {
    }

    ProtocolCallContext callContext(0);
    m_pRelayClient->CallRequestProbeInitiationExt(&callContext, rTargetURLs, rSourceURL);
}

void RVNATRelay::CheckCurrentPublicPort(CallContext* pCallContext, const InetAddress& rAddress, bool bForce)
{
    NATCheckClient natCheckClient(rAddress);
    natCheckClient.DetectNATCurrentPortMapping(pCallContext, bForce);
}

void RVNATRelay::DetectSimplifiedNATProperties(CallContext* pCallContext, const InetAddress& rAddress)
{
    NATCheckClient natCheckClient(rAddress);
    natCheckClient.DetectSimplifiedNATProperties(pCallContext);
}

void RVNATRelay::RequestProbe(const StationURL& rTargetURL)
{
    if (m_pEngine != 0)
        m_pEngine->ProcessRequestProbe(rTargetURL);
}

void RVNATRelay::ReportNATTraversalResult(const unsigned int& rConnectionID, const bool& rSucceeded,
                                          const unsigned int& rRVConnectionID)
{
    ProtocolCallContext callContext(0);
    m_pRelayClient->CallReportNATTraversalResult(&callContext, rConnectionID, rSucceeded, rRVConnectionID);
}

void RVNATRelay::ReportNATProperties(unsigned int uiMapping, unsigned int uiFiltering, unsigned int uiPortIncrease)
{
    ProtocolCallContext callContext(0);
    m_pRelayClient->CallReportNATProperties(&callContext, uiMapping, uiFiltering, uiPortIncrease);
}

bool RVNATRelay::GetRelaySignatureKey(CallContext* pCallContext, RelaySignatureKey* pKey)
{
    JobProtocolCallContextCallback* pJob = new JobProtocolCallContextCallback(Job::JOB_TYPE_2, L"GetRelaySignatureKey",
                                                                              pCallContext);
    if (!m_pRelayClient->CallGetRelaySignatureKey(pJob->GetProtocolCallContext(), &pKey->m_relayMode,
                                                  &pKey->m_currentUTCTime, &pKey->m_relayAddress, &pKey->m_relayPort,
                                                  &pKey->m_relayAddressType, &pKey->m_gameServerID)
        || !pJob->StartJob())
    {
        pJob->ReleaseRef();
        return false;
    }

    return true;
}

void RVNATRelay::ReportNATTraversalResultDetail(const unsigned int& rConnectionID, const bool& rSucceeded,
                                                const NATTraversalResult& rResult, unsigned int& rRVConnectionID)
{
    ProtocolCallContext callContext(0);
    m_pRelayClient->CallReportNATTraversalResultDetail(&callContext, rConnectionID, rSucceeded, rResult,
                                                       rRVConnectionID);
}

void RVNATRelay::UpdateConnectionState(unsigned int uiConnectionID, const StationURL& rStationURL)
{
    m_pEngine->m_uiConnectionID = uiConnectionID;
    m_echo.SetStreamURL(rStationURL);
}

void RVNATRelay::AssociateEngine(NATTraversalEngine* pEngine)
{
    if (m_pEngine != 0)
    {
        m_pEngine->UnregisterRelay();
        m_pEngine->UnregisterEcho();
        m_pEngine->UnregisterConnectivityTester();
    }

    m_pEngine = pEngine;
    if (pEngine != 0)
    {
        m_pEngine->RegisterRelay(this);
        m_pEngine->RegisterEcho(&m_echo);
        m_pEngine->RegisterConnectivityTester(&m_connectivityTester);
    }
}

}
}
