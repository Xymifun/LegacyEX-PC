#include "nn/nex/ServiceClient.hpp"

#include "nn/nex/ClientProtocol.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/Protocol.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

ServiceClient::ServiceClient(unsigned int uiServiceID)
    : m_uiServiceID(uiServiceID),
      m_pCredentials(0),
      m_bLocalLoopback(false)
{
    if (RVClientCore::GetInstance() != 0)
        RVClientCore::GetInstance()->AddRef();
    else
        RVClientCore::AcquireInstance();
}

ServiceClient::~ServiceClient()
{
    SetCredentials(0);
    RVClientCore::GetInstance()->ReleaseRef();
}

void ServiceClient::UseLocalLoopback(unsigned int, unsigned int)
{
}

bool ServiceClient::Bind(Credentials* pCredentials)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());

    if (!PrepareForConnection(pCredentials))
        return false;

    ConnectionStateHasChanged();
    return true;
}

bool ServiceClient::Unbind()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());

    ConnectionStateHasChanged();
    SetDefaultCredentials(0);
    return true;
}

bool ServiceClient::BindLocal(Credentials* pCredentials)
{
    m_bLocalLoopback = true;
    bool bResult = Bind(pCredentials);
    UseLocalLoopback(pCredentials->GetPrincipalID(), pCredentials->m_state);
    return bResult;
}

void ServiceClient::ConnectionStateHasChanged()
{
}

bool ServiceClient::IsConnected() const
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());

    if (m_pCredentials == 0)
        return false;

    EndPoint* pConnection = GetConnection(SECURE_CONNECTION_TYPE);
    return pConnection != 0 && pConnection->IsConnected();
}

bool ServiceClient::SetDependentRvConnection(ProtocolCallContext* pCallContext) const
{
    Credentials* pCredentials = pCallContext->GetCredentials() != 0 ? pCallContext->GetCredentials() : m_pCredentials;

    if (pCredentials == 0)
        return false;

    EndPoint* pConnection = pCredentials->GetConnection(SECURE_CONNECTION_TYPE);
    if (pConnection == 0)
        return false;

    pCallContext->SetDependentConnection(pConnection, pConnection->GetConnectionID());
    return true;
}

bool ServiceClient::IsFaulty() const
{
    if (m_pCredentials == 0)
        return false;

    EndPoint* pConnection = GetConnection(SECURE_CONNECTION_TYPE);
    return pConnection != 0 && pConnection->IsFaulty();
}

StreamManager* ServiceClient::GetStreamManager() const
{
    if (m_pCredentials == 0)
        return 0;

    return m_pCredentials->m_streamManager;
}

void ServiceClient::SetCredentials(Credentials* pCredentials)
{
    if (m_pCredentials != 0)
    {
        m_pCredentials->ReleaseRef();
        m_pCredentials = 0;
    }

    if (pCredentials == 0)
        return;

    m_pCredentials = pCredentials;
    m_pCredentials->AddRef();
}

void ServiceClient::SetDefaultCredentials(Credentials* pCredentials)
{
    UpdateProtocolsDefaultCredentials(pCredentials);
    SetCredentials(pCredentials);
}

bool ServiceClient::PrepareForConnection(Credentials* pCredentials)
{
    if (pCredentials == 0)
        return false;

    SetDefaultCredentials(pCredentials);
    return true;
}

bool ServiceClient::RegisterProtocol(Protocol* pProtocol)
{
    return RegisterExtraProtocol(pProtocol);
}

bool ServiceClient::RegisterExtraProtocol(Protocol* pProtocol)
{
    return RegisterExtraProtocol(pProtocol, pProtocol->GetProtocolID());
}

bool ServiceClient::RegisterExtraProtocol(Protocol* pProtocol, unsigned short uiProtocolID)
{
    ClientProtocol* pClientProtocol = static_cast<ClientProtocol*>(pProtocol);

    pClientProtocol->SetDefaultCredentials(m_pCredentials);
    pClientProtocol->SetProtocolID(uiProtocolID);
    return GetProtocolRegistry()->RegisterProtocol(uiProtocolID, pClientProtocol->CreateResponder());
}

ProtocolRegistry* ServiceClient::GetProtocolRegistry() const
{
    return RVClientCore::GetInstance()->GetProtocolRegistry();
}

bool ServiceClient::UnregisterProtocol(Protocol*)
{
    return true;
}

EndPoint* ServiceClient::GetConnection(unsigned short uiConnectionType) const
{
    if (m_bLocalLoopback || m_pCredentials == 0)
        return 0;

    return m_pCredentials->GetConnection(uiConnectionType);
}

}
}
