#include "nn/nex/ClientStreamManager.hpp"

#include "nn/nex/ClientProtocolRequestBroker.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/JobProcessProtocolMessage.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

ClientStreamManager::ClientStreamManager(Stream::Type type)
    : StreamManager(type, RootTransport::GetInstance()),
      m_pCredentials(0)
{
    m_pProtocolRequestBroker = new ClientProtocolRequestBroker;
}

ClientStreamManager::~ClientStreamManager()
{
    SetCredentials(0);
}

void ClientStreamManager::Receive(EndPoint* pEndPoint, Buffer* pBuffer, unsigned char)
{
    Scheduler::GetInstance()->Queue(new JobProcessProtocolMessage(m_pProtocolRequestBroker, pEndPoint, pBuffer), false);
}

void ClientStreamManager::FaultDetection(EndPoint* pEndPoint, unsigned int uiError)
{
    static_cast<ClientProtocolRequestBroker*>(m_pProtocolRequestBroker)->FaultDetection(pEndPoint, uiError);
}

void ClientStreamManager::PeerDisconnected(EndPoint* pEndPoint)
{
    static_cast<ClientProtocolRequestBroker*>(m_pProtocolRequestBroker)->PeerDisconnected(pEndPoint);
}

void ClientStreamManager::SetCredentials(Credentials* pCredentials)
{
    if (m_pCredentials != 0)
    {
        m_pCredentials->ReleaseRef();
        m_pCredentials = 0;
    }

    if (pCredentials != 0)
    {
        m_pCredentials = pCredentials;
        m_pCredentials->AddRef();
    }
}

}
}
