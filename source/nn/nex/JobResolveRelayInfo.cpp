#include "nn/nex/JobResolveRelayInfo.hpp"

#include "nn/nex/CallContext.hpp"
#include "nn/nex/ConnectivityManager.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/NATProperties.hpp"
#include "nn/nex/NATRelayInterface.hpp"
#include "nn/nex/NATTraversalEngine.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/Relay.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum
{
    RESOLVE_RELAY_INFO_ABORTED_RESULT = static_cast<int>(0x80010004u)
};

const unsigned short DEFAULT_RELAY_INFO_PORT = 12525;
const unsigned int NAT_PORT_INCREMENT_SEQUENTIAL = 1;

}

void JobResolveRelayInfo::FailResolvedRelayInfo()
{
    if (RootTransport::GetInstance() != 0)
        RootTransport::GetInstance()->GetRelay()->EnableRelay(false);
}

JobResolveRelayInfo* JobResolveRelayInfo::CreateAndStartJob(CallContext* pCallContext)
{
    if (!ConnectivityManager::GetInstance()->m_natTraversalEnabled)
    {
        pCallContext->InitiateCall();
        pCallContext->SignalFailure(qResult(RESOLVE_RELAY_INFO_ABORTED_RESULT));
        return 0;
    }

    if (!RootTransport::GetInstance()->GetRelay()->IsEnabled())
    {
        pCallContext->InitiateCall();
        pCallContext->SignalFailure(qResult(RESOLVE_RELAY_INFO_ABORTED_RESULT));
        return 0;
    }

    NATTraversalEngine* pEngine = Network::GetInstance()->GetNATTraversalEngine();
    if (pEngine == 0)
    {
        FailResolvedRelayInfo();
        pCallContext->InitiateCall();
        pCallContext->SignalFailure(qResult(RESOLVE_RELAY_INFO_ABORTED_RESULT));
        return 0;
    }

    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    JobResolveRelayInfo* pJob = new JobResolveRelayInfo(pCallContext);
    if (!pEngine->m_pRelay->GetRelaySignatureKey(pJob->GetCallContext(), &pJob->m_key) || !pJob->StartJob())
    {
        pJob->ReleaseRef();
        FailResolvedRelayInfo();
        pCallContext->InitiateCall();
        pCallContext->SignalFailure(qResult(RESOLVE_RELAY_INFO_ABORTED_RESULT));
        return 0;
    }

    RootTransport::GetInstance()->GetRelay()->Clean();
    pJob->AddRef();
    return pJob;
}

bool JobResolveRelayInfo::SetRelayAddress()
{
    if (Network::GetInstance() == 0 || RootTransport::GetInstance() == 0)
        return false;

    switch (m_key.m_relayAddressType)
    {
    case RELAY_ADDRESS_IP:
        Network::GetInstance()->GetNATTraversalEngine()->m_relayServerAddress = m_key.m_relayAddress;
        break;

    case RELAY_ADDRESS_HOST_NAME:
        Network::GetInstance()->GetNATTraversalEngine()->m_relayServerAddress =
            RootTransport::GetInstance()->m_relayAddress.GetAddressStr();
        break;

    default:
        FailResolvedRelayInfo();
        return false;
    }

    if (m_key.m_relayPort != 0)
    {
        Network::GetInstance()->GetNATTraversalEngine()->m_uiRelayServerPort = m_key.m_relayPort;
        return true;
    }

    Network::GetInstance()->GetNATTraversalEngine()->m_uiRelayServerPort = DEFAULT_RELAY_INFO_PORT;
    return true;
}

void JobResolveRelayInfo::ResolvedRelayInfo()
{
    if (Network::GetInstance() == 0 || RootTransport::GetInstance() == 0)
        return;

    if (m_key.m_relayMode <= RELAY_MODE_DISABLED)
    {
        FailResolvedRelayInfo();
        return;
    }

    switch (m_key.m_relayMode)
    {
    case RELAY_MODE_ADDRESS_ONLY:
        if (!SetRelayAddress())
            return;
        break;

    case RELAY_MODE_WHEN_NEEDED:
        if (!SetRelayAddress())
            return;

        if (RootTransport::GetInstance()->GetNATProperties().GetNATMapping()
            != NATProperties::MAPPING_ENDPOINT_DEPENDENT)
        {
            Network::GetInstance()->GetNATTraversalEngine()->m_bUseRelayServer = true;
        }
        break;

    case RELAY_MODE_ALWAYS:
        if (!SetRelayAddress())
            return;

        Network::GetInstance()->GetNATTraversalEngine()->m_bUseRelayServer = true;
        break;

    default:
        FailResolvedRelayInfo();
        return;
    }

    RelayMessage* pRelayMessage = Network::GetInstance()->GetMessageSelector().GetRelayMessage();
    pRelayMessage->m_timeStampOrigin.m_uiBaseTime = m_key.m_currentUTCTime.ToEpochTime();
    pRelayMessage->m_timeStampOrigin.m_uiStartTime = Time::GetTime();
    RootTransport::GetInstance()->GetRelay()->SetGameServerID(m_key.m_gameServerID);

    NATProperties properties = RootTransport::GetInstance()->GetNATProperties();
    if (properties.GetNATMapping() == NATProperties::MAPPING_ENDPOINT_DEPENDENT
        && properties.GetNATMappingPortIncrement() != NAT_PORT_INCREMENT_SEQUENTIAL)
    {
        Network::GetInstance()->GetNATTraversalEngine()->m_bUseRelayServer = true;
    }

    ConnectivityManager::SetRelayPublicAddress();
}

}
}
