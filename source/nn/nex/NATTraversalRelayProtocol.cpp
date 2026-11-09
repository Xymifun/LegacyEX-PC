#include "nn/nex/NATTraversalRelayProtocol.hpp"

#include "nn/nex/ProtocolRequestBrokerInterface.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned short NAT_TRAVERSAL_PROTOCOL_ID = 3;

}

NATTraversalRelayProtocol::NATTraversalRelayProtocol() : _Proto_NATTraversalProtocolServer(1), m_reportInternalClient(0)
{
    m_pProtocolClient = 0;
    m_pProtocolClient = new NATTraversalProtocolClient(0);
    m_pProtocolClient->SetProtocolID(NAT_TRAVERSAL_PROTOCOL_ID);
}

NATTraversalRelayProtocol::~NATTraversalRelayProtocol()
{
    if (m_pProtocolClient != 0)
    {
        delete m_pProtocolClient;
        m_pProtocolClient = 0;
    }
}

void NATTraversalRelayProtocol::CloneRelayProtocol(NATTraversalRelayProtocol* pTarget) const
{
    pTarget->m_pProtocolClient = m_pProtocolClient;
    pTarget->m_pCloneableProtocol = m_pCloneableProtocol->Clone();
}

void NATTraversalRelayProtocol::RegisterProtocols(ProtocolRequestBrokerInterface* pRequestBroker)
{
    SetProtocolID(NAT_TRAVERSAL_PROTOCOL_ID);
    m_pProtocolClient = new NATTraversalProtocolClient(0);
    m_pProtocolClient->SetProtocolID(NAT_TRAVERSAL_PROTOCOL_ID);
    pRequestBroker->RegisterProtocol(m_pProtocolClient);
    pRequestBroker->RegisterProtocol(this);
}

void NATTraversalRelayProtocol::UnregisterProtocols(ProtocolRequestBrokerInterface* pRequestBroker)
{
    pRequestBroker->UnregisterProtocol(m_pProtocolClient);
    if (m_pProtocolClient != 0)
        delete m_pProtocolClient;
    m_pProtocolClient = 0;
    pRequestBroker->UnregisterProtocol(this);
}

void NATTraversalRelayProtocol::SetClientConnection(EndPoint* pEndPoint)
{
    if (m_pProtocolClient != 0)
        m_pProtocolClient->SetOutgoingConnection(pEndPoint);
}

void NATTraversalRelayProtocol::SetDefaultCredentials(Credentials* pCredentials)
{
    m_pProtocolClient->SetDefaultCredentials(pCredentials);
}

void NATTraversalRelayProtocol::InitiateProbe(const StationURL&)
{
}

void NATTraversalRelayProtocol::RequestProbeInitiation(const qList<StationURL>&)
{
}

void NATTraversalRelayProtocol::RequestProbeInitiationExt(const qList<StationURL>&, const StationURL&)
{
}

void NATTraversalRelayProtocol::ReportNATTraversalResult(const unsigned int&, const bool&, const unsigned int&)
{
}

void NATTraversalRelayProtocol::ReportNATProperties(const unsigned int&, const unsigned int&, const unsigned int&)
{
}

void NATTraversalRelayProtocol::GetRelaySignatureKey(int*, DateTime*, String*, unsigned short*, int*, unsigned int*)
{
}

void NATTraversalRelayProtocol::ReportNATTraversalResultDetail(const unsigned int&, const bool&, const int&,
                                                               const unsigned int&)
{
}

}
}
