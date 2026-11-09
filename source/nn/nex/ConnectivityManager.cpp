#include "nn/nex/ConnectivityManager.hpp"

#include <nn/ndm.h>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/ConnectionManager.hpp"
#include "nn/nex/ConnectivityTester.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/InstanceTable.hpp"
#include "nn/nex/NATProperties.hpp"
#include "nn/nex/NATRelayInterface.hpp"
#include "nn/nex/NATTraversalEngine.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/PacketBufferManager.hpp"
#include "nn/nex/Relay.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Router.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/StationProbeList.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/Stream.hpp"
#include "nn/nex/qList.hpp"
#include "nn/nex/qProtectedList.hpp"

void StartNATSessionCallback(nn::nex::CallContext* pCallContext, const nn::nex::UserContext* pUserContext);

namespace
{

bool IsRendezVousRunning()
{
    return nn::nex::RootTransport::GetInstance()->GetStreamManager()->IsStreamTypeRegistered(
        nn::nex::Stream::RendezVousSecure);
}

}

namespace nn
{
namespace nex
{

enum
{
    RESULT_CORE_SUCCESS = 0x10001,
    RESULT_CORE_UNKNOWN = static_cast<int>(0x80010001u),
    RESULT_CORE_OPERATION_ABORTED = static_cast<int>(0x80010004u)
};

enum
{
    STATION_TYPE_PUBLIC = 2
};

const unsigned int BOSS_DAEMONS = 3;

bool ConnectivityManager::s_suspendBossDaemonOnNATSession = true;
bool ConnectivityManager::s_enableRelayOnNATSession = false;

ConnectivityManager::ConnectivityManager() : PseudoSingleton(ClassID)
{
    m_natSessionStarted = false;
    m_natTraversalEnabled = true;
    m_portNumber = RootTransport::GetWellKnownPortNumber();
    m_reservedPortNumber = 0;
    m_connectivityTester = 0;
    m_callContextId = 0;
    m_probeLifetime = 0;
    m_bossDaemonSuspendCount = 0;
    m_packetBufferManagerInitialized = false;
}

ConnectivityManager::~ConnectivityManager()
{
    if (m_callContextId != 0)
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());

        CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_callContextId);
        if (pCallContext != 0 && pCallContext->GetState() == CallContext::CallInProgress)
            pCallContext->SignalFailure(qResult(RESULT_CORE_OPERATION_ABORTED));
    }

    if (s_suspendBossDaemonOnNATSession && m_bossDaemonSuspendCount != 0)
    {
        ndm::ResumeDaemons(BOSS_DAEMONS);
        ndm::Finalize();
        --m_bossDaemonSuspendCount;
    }

    if (m_packetBufferManagerInitialized)
    {
        PacketBufferManager::GetInstance()->Finalize();
        m_packetBufferManagerInitialized = false;
    }
}

ConnectivityManager* ConnectivityManager::GetInstance()
{
    return GetInstanceWithoutNetworkCheck();
}

ConnectivityManager* ConnectivityManager::GetInstanceWithoutNetworkCheck()
{
    unsigned int contextId = g_usingInstantiationContext;
    if (g_usingInstantiationContext)
        contextId = g_instantiationContextVariable->GetRef();

    return static_cast<ConnectivityManager*>(g_instanceTable.FindInstance(ClassID, contextId));
}

bool ConnectivityManager::RegisterConnectivityTester(ConnectivityTester* connectivityTester)
{
    if (m_connectivityTester == 0)
    {
        m_connectivityTester = connectivityTester;
        return true;
    }

    return false;
}

bool ConnectivityManager::UnregisterConnectivityTester()
{
    if (m_connectivityTester != 0)
    {
        m_connectivityTester = 0;
        return true;
    }

    return false;
}

bool ConnectivityManager::ProbeStations(CallContext* callContext, StationProbeList* stationProbeList,
                                        unsigned int timeout, bool useRelay)
{
    if (m_connectivityTester == 0)
        return false;

    if (timeout == 0)
        timeout = nn::nex::ConnectionManager::s_iNATTraversalTimeout;

    if (IsRendezVousRunning() && !m_natSessionStarted)
        return false;

    stationProbeList->Reset();
    return m_connectivityTester->ProbeStations(callContext, stationProbeList, timeout, useRelay, Job::JOB_TYPE_2);
}

bool ConnectivityManager::StartNATSessionInternal(CallContext* callContext)
{
    if (RootTransport::GetInstance() == 0 || m_natSessionStarted)
        return false;

    if (RootTransport::GetInstance()->GetStreamManager()->GetNBStreamByStreamType(Stream::Unknown9) != 0)
        return false;

    if (m_callContextId != 0)
        return false;

    if (!callContext->InitiateCall())
        return false;

    m_callContextId = callContext->m_uiCallId;

    do
    {
        m_portNumber = RootTransport::GetNextPortNumber();
    } while (m_portNumber == m_reservedPortNumber);

    if (RootTransport::GetInstance()->GetNATProperties().GetNATMapping() == NATProperties::MAPPING_UNKNOWN)
    {
        NATRelayInterface* pRelay = Network::GetInstance()->GetNATTraversalEngine()->m_pRelay;
        if (pRelay == 0)
        {
            callContext->SignalFailure(qResult(RESULT_CORE_UNKNOWN));
            return false;
        }

        CallContext* pCheckContext = new CallContext();
        pCheckContext->SetFlag(CALL_CONTEXT_FLAG_SELF_RELEASE);
        pCheckContext->RegisterCompletionCallback(StartNATSessionCallback, UserContext(this), true);

        InetAddress address;
        address.SetPortNumber(m_portNumber);
        pRelay->DetectSimplifiedNATProperties(pCheckContext, address);
    }
    else if (RootTransport::GetInstance()->GetNATProperties().GetNATMapping()
             == NATProperties::MAPPING_ENDPOINT_DEPENDENT)
    {
        OnStartNATSession(0);
    }
    else
    {
        NATRelayInterface* pRelay = Network::GetInstance()->GetNATTraversalEngine()->m_pRelay;
        if (pRelay == 0)
        {
            callContext->SignalFailure(qResult(RESULT_CORE_UNKNOWN));
            return false;
        }

        CallContext* pCheckContext = new CallContext();
        pCheckContext->SetFlag(CALL_CONTEXT_FLAG_SELF_RELEASE);
        pCheckContext->RegisterCompletionCallback(StartNATSessionCallback, UserContext(this), true);

        InetAddress address;
        address.SetPortNumber(m_portNumber);
        pRelay->CheckCurrentPublicPort(pCheckContext, address, false);
    }

    if (s_suspendBossDaemonOnNATSession
        && (RootTransport::GetInstance()->GetNATProperties().GetNATMapping()
                != NATProperties::MAPPING_ENDPOINT_DEPENDENT
            || m_natSessionStarted)
        && ndm::Initialize().IsSuccess())
    {
        ndm::SuspendDaemons(BOSS_DAEMONS);
        ++m_bossDaemonSuspendCount;
    }

    if (s_enableRelayOnNATSession)
        RootTransport::GetInstance()->GetRelay()->EnableRelay(true);

    return true;
}

bool ConnectivityManager::StopNATSession()
{
    if (RootTransport::GetInstance() == 0 || !m_natSessionStarted)
        return false;

    RootTransport::GetInstance()->GetStreamManager()->GetNBStreamByStreamType(Stream::Unknown9);
    if (!RootTransport::GetInstance()->StopListen(m_portNumber))
        return false;

    Network::GetInstance()->GetNATTraversalEngine()->ClearProbeList();
    Network::GetInstance()->GetNATTraversalEngine()->StopSendDummyPacket();
    m_natSessionStarted = false;

    if (s_suspendBossDaemonOnNATSession && m_bossDaemonSuspendCount != 0)
    {
        ndm::ResumeDaemons(BOSS_DAEMONS);
        ndm::Finalize();
        --m_bossDaemonSuspendCount;
    }

    if (m_packetBufferManagerInitialized)
    {
        PacketBufferManager::GetInstance()->Finalize();
        m_packetBufferManagerInitialized = false;
    }

    RootTransport::GetInstance()->GetRelay()->Clean();
    RootTransport::GetInstance()->GetRouter()->Clean();

    Network::GetInstance()->GetNATTraversalEngine()->m_bUseRelayServer = false;
    if (s_enableRelayOnNATSession)
        RootTransport::GetInstance()->GetRelay()->EnableRelay(false);

    return true;
}

bool ConnectivityManager::InitiateProbes(StationProbeList& stationProbeList)
{
    qList<StationURL> localURLs;
    Network::GetInstance()->GetLocalURLs(&localURLs);

    for (StationProbeList::iterator probe = stationProbeList.begin(); probe != stationProbeList.end(); ++probe)
    {
        qList<StationURL> targetURLs(probe->m_contactInfo.GetURLs());
        Network::GetInstance()->SortAndFilterTargetURLs(&targetURLs);
        Network::GetInstance()->GetNATTraversalEngine()->PrepareNATTraversal(targetURLs);
    }

    return true;
}

void ConnectivityManager::SetSuspendBossDaemonOnNATSession(bool suspend)
{
    s_suspendBossDaemonOnNATSession = suspend;
}

bool ConnectivityManager::GetSuspendBossDaemonOnNATSession()
{
    return s_suspendBossDaemonOnNATSession;
}

void ConnectivityManager::SetRelayPublicAddress()
{
    qList<StationURL> localURLs;
    {
        qProtectedList<StationURL>& rLocalURLs = Network::GetInstance()->GetLocalURLs();
        ScopedCS lock(&rLocalURLs.GetCriticalSection());
        localURLs = rLocalURLs.GetList();
    }

    for (qList<StationURL>::iterator it = localURLs.begin(); it != localURLs.end(); ++it)
    {
        if ((it->GetType() & STATION_TYPE_PUBLIC) != 0 && it->GetRVConnectionID() != 0)
            RootTransport::GetInstance()->GetRelay()->SetSrcPublicAddress(*it);
    }
}

void ConnectivityManager::OnStartNATSession(CallContext* callContext)
{
    bool useInterfacePort = false;
    if (RootTransport::GetInstance() != 0 && RootTransport::GetInstance()->GetNATProperties().GetInterfacePort() != 0
        && RootTransport::GetInstance()->GetNATProperties().GetNATMapping()
               != NATProperties::MAPPING_ENDPOINT_DEPENDENT)
    {
        useInterfacePort = true;
    }

    if (useInterfacePort)
        m_portNumber = RootTransport::GetInstance()->GetNATProperties().GetInterfacePort();

    RootTransport::SetWellKnownPortNumber(m_portNumber);

    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());

    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_callContextId);

    if ((callContext != 0 && callContext->GetState() != CallContext::CallSucceeded) || Network::GetInstance() == 0
        || RootTransport::GetInstance() == 0
        || !RootTransport::GetInstance()->StartListenReuse(m_portNumber, 0, true, 0, true))
    {
        m_natSessionStarted = false;

        if (s_suspendBossDaemonOnNATSession && m_bossDaemonSuspendCount != 0)
        {
            ndm::ResumeDaemons(BOSS_DAEMONS);
            ndm::Finalize();
            --m_bossDaemonSuspendCount;
        }

        if (pCallContext != 0)
        {
            if (callContext->GetState() == CallContext::CallFailed)
                pCallContext->SignalFailure(callContext->GetResult());
            else
                pCallContext->SetStateImpl(CallContext::CallFailed, qResult(RESULT_CORE_UNKNOWN), false);
        }

        if (s_enableRelayOnNATSession && RootTransport::GetInstance() != 0)
            RootTransport::GetInstance()->GetRelay()->EnableRelay(false);

        m_callContextId = 0;
        return;
    }

    Network::GetInstance()->GetNATTraversalEngine()->StartSendDummyPacket();
    m_natSessionStarted = true;

    if (pCallContext != 0)
        pCallContext->SetStateImpl(CallContext::CallSucceeded, qResult(RESULT_CORE_SUCCESS), true);

    if (!m_packetBufferManagerInitialized)
    {
        PacketBufferManager::GetInstance()->Initialize();
        m_packetBufferManagerInitialized = true;
    }

    m_callContextId = 0;
}

}
}

void StartNATSessionCallback(nn::nex::CallContext* pCallContext, const nn::nex::UserContext* pUserContext)
{
    nn::nex::ConnectivityManager* pConnectivityManager = static_cast<nn::nex::ConnectivityManager*>(
        pUserContext->GetPointer());
    if (pConnectivityManager == nn::nex::ConnectivityManager::GetInstanceWithoutNetworkCheck())
        pConnectivityManager->OnStartNATSession(pCallContext);
}
