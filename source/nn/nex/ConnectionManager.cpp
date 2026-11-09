#include "nn/nex/ConnectionManager.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/ConnectionOrientedStream.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/JobConnectEndPoint.hpp"
#include "nn/nex/PseudoSingleton.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Router.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/String.hpp"

namespace __N_19_OnlineCore_Blob_cpp_9428d484
{

void DisconnectCallback(nn::nex::EndPoint* pEndPoint, nn::nex::qResult result, const nn::nex::UserContext* pUserContext)
{
    nn::nex::CriticalSection& csScheduler = nn::nex::Scheduler::GetInstance()->GetCriticalSection();
    csScheduler.Enter();

    unsigned int uiCallId = reinterpret_cast<unsigned int>(pUserContext->GetPointer());
    nn::nex::CallContext* pCallContext = nn::nex::Core::GetInstance()->GetCallContextRegister()->GetCallContext(
        uiCallId);
    if (pCallContext != 0)
    {
        if (result == true)
        {
            pCallContext->SignalSuccess(result);
            pEndPoint->Close();
            csScheduler.Leave();
            return;
        }
        pCallContext->SignalFailure(result);
    }

    pEndPoint->Close();
    csScheduler.Leave();
}

}

namespace nn
{
namespace nex
{

int ConnectionManager::s_iNATTraversalTimeout = 60000;
unsigned int ConnectionManager::s_uiDisconnectTimeout = 15000;

ConnectionManager::ConnectionManager(EndPointEventHandler* pEndPointEventHandler, Job::JobType jobType, bool bFlag,
                                     bool bRoutingEnabled)
    : m_component(String(L"ConnectionManager")),
      m_pEndPointEventHandler(pEndPointEventHandler),
      m_flag64(bFlag),
      m_bRoutingEnabled(bRoutingEnabled),
      m_jobType(jobType)
{
    m_bFilterEnabled = false;
    m_pConnectionFilter = 0;
    unsigned int uiContextId = g_usingInstantiationContext;
    if (g_usingInstantiationContext)
        uiContextId = g_instantiationContextVariable->GetRef();
    m_uiContextId = uiContextId;

    m_component.Initialize();
    RootTransport::GetInstance()->GetRouter()->PeriodicHouseCleaning();
    RootTransport::GetInstance()->GetRouter()->EnableRouting(false);
    if (m_bRoutingEnabled)
        RootTransport::GetInstance()->GetRouter()->EnableRouting(true);
    else
        RootTransport::GetInstance()->GetRouter()->EnableRouting(false);
}

ConnectionManager::~ConnectionManager()
{
}

void ConnectionManager::Terminate()
{
    m_component.Terminate();
}

bool ConnectionManager::IsTerminated() const
{
    return m_component.IsTerminated();
}

void ConnectionManager::AttachStream(ConnectionOrientedStream* pStream, bool)
{
    pStream->RegisterEventHandler(this);
    m_streamTable.GetStreams().push_back(pStream);
}

void ConnectionManager::RegisterConnectionFilter(ConnectionFilter pFilter, UserContext userContext)
{
    m_pConnectionFilter = pFilter;
    m_connectionFilterContext = userContext;
}

bool ConnectionManager::ConnectionRequest(ConnectionOrientedStream* pStream, const StationURL* pStationURL,
                                          Buffer* pBuffer, EndPoint* pEndPoint)
{
    PseudoSingleton::SetContextIfRequired(m_uiContextId);
    if (!m_bFilterEnabled)
        return false;
    if (!FilterConnectionRequest(pStream, pStationURL, pBuffer, pEndPoint))
        return false;

    pEndPoint->RegisterEventHandler(m_pEndPointEventHandler);
    return true;
}

bool ConnectionManager::FilterConnectionRequest(ConnectionOrientedStream*, const StationURL*, Buffer* pBuffer,
                                                EndPoint* pEndPoint)
{
    if (m_pConnectionFilter != 0)
    {
        Buffer response;
        if (!m_pConnectionFilter(m_connectionFilterContext, pBuffer, &response, pEndPoint))
            return false;
        *pBuffer = response;
    }
    return true;
}

EndPoint* ConnectionManager::OpenEndPoint(const StationURL& rStationURL)
{
    ConnectionOrientedStream* pStream = m_streamTable.FindStream(&rStationURL);
    if (pStream == 0)
        return 0;

    EndPoint* pEndPoint = pStream->OpenEndPoint(&rStationURL);
    if (pEndPoint != 0)
        pEndPoint->RegisterEventHandler(m_pEndPointEventHandler);
    return pEndPoint;
}

bool ConnectionManager::Disconnect(CallContext* pCallContext, EndPoint* pEndPoint)
{
    CriticalSection& csScheduler = Scheduler::GetInstance()->GetCriticalSection();
    csScheduler.Enter();

    if (!pCallContext->InitiateCall())
    {
        csScheduler.Leave();
        return false;
    }

    UserContext userContext(reinterpret_cast<void*>(pCallContext->m_uiCallId));
    qResult result = pEndPoint->_Disconnect(__N_19_OnlineCore_Blob_cpp_9428d484::DisconnectCallback, userContext,
                                            s_uiDisconnectTimeout);
    if (!result)
    {
        pEndPoint->Close();
        csScheduler.Leave();
        return false;
    }

    csScheduler.Leave();
    return true;
}

void ConnectionManager::SetRouterAddress(const InetAddress& rAddress)
{
    if (m_bRoutingEnabled)
        m_routerAddress = rAddress;
}

void ConnectionManager::ConfigureEndPointForRouting(EndPoint* pEndPoint)
{
    if (m_routerAddress.GetAddress() != 0)
    {
        pEndPoint->SetMaxSilenceTime(0xFFFFFFFF);
        pEndPoint->DisableKeepAlive();
        RootTransport::GetInstance()->GetRouter()->AddRoute(*pEndPoint->m_stationURL.GetInetAddress(), m_routerAddress,
                                                            true);
    }
}

void ConnectionManager::ConstructAndAssociateRoutingURL(unsigned short uiPort, StationURL oStationURL,
                                                        StationURL* pRoutingURL)
{
    Router* pRouter = RootTransport::GetInstance()->GetRouter();
    InetAddress target;
    InetAddress routing;
    target = *oStationURL.GetInetAddress();
    pRouter->ConstructAndAssociateRoutingAddress(uiPort, target, &routing);
    pRoutingURL->SetInetAddress(&routing);
}

bool ConnectionManager::ConnectImpl(CallContext* pCallContext, Buffer* pRequestBuffer, Buffer* pResponseBuffer,
                                    const qList<StationURL>& rURLs, EndPoint** ppEndPoint, unsigned int uiTimeout,
                                    bool bFlag, JobConnectEndPoint** ppJob)
{
    CriticalSection& csScheduler = Scheduler::GetInstance()->GetCriticalSection();
    csScheduler.Enter();

    if (!pCallContext->InitiateCall())
    {
        csScheduler.Leave();
        return false;
    }

    JobConnectEndPoint* pJob = new JobConnectEndPoint(m_jobType, this, pCallContext->m_uiCallId, pRequestBuffer,
                                                      pResponseBuffer, rURLs, ppEndPoint, uiTimeout, bFlag);
    Scheduler::GetInstance()->Queue(pJob, false);
    if (ppJob != 0)
        *ppJob = pJob;

    csScheduler.Leave();
    return true;
}

}
}
