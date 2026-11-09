#include "nn/nex/NATTraversalRelayClient.hpp"

#include "nn/nex/Credentials.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/NATRelayInterface.hpp"
#include "nn/nex/NATTraversalProtocolClient.hpp"
#include "nn/nex/NATTraversalRelayProtocol.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/StreamManager.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int NAT_TRAVERSAL_SERVICE_ID = 3;
const unsigned short NAT_TRAVERSAL_PROTOCOL_ID = 3;
const unsigned short NAT_TRAVERSAL_CONNECTION_TYPE = 3;
const unsigned int RELAY_PROTOCOL_FLAGS = 8;

}

class NATTraversalRelayProtocolImpl : public NATTraversalRelayProtocol
{
public:
    virtual void InitiateProbe(const StationURL& rStationToProbe);
};

void NATTraversalRelayProtocolImpl::InitiateProbe(const StationURL& rStationToProbe)
{
    if (m_pRelayClient != 0)
        m_pRelayClient->m_pRelayInterface->RequestProbe(rStationToProbe);
}

NATTraversalRelayClient::NATTraversalRelayClient() : ServiceClient(NAT_TRAVERSAL_SERVICE_ID)
{
    m_pRelayInterface = 0;
    m_pRelayProtocol = 0;
}

NATTraversalRelayClient::~NATTraversalRelayClient()
{
    if (m_pRelayInterface != 0)
    {
        m_pRelayInterface->UnregisterRelayClient();
        m_pRelayInterface = 0;
    }
    if (m_pRelayProtocol != 0)
        delete m_pRelayProtocol;
}

void NATTraversalRelayClient::Init()
{
    NATTraversalRelayProtocol* pRelayProtocol = new NATTraversalRelayProtocol();
    GetProtocolRegistry()->RegisterProtocol(NAT_TRAVERSAL_PROTOCOL_ID, pRelayProtocol);
    NATTraversalProtocolClient* pProtocolClient = new NATTraversalProtocolClient();
    GetProtocolRegistry()->RegisterProtocol(NAT_TRAVERSAL_PROTOCOL_ID, pProtocolClient);
    m_pRelayProtocol = CreateNATTraversalRelayProtocol();
    m_pRelayProtocol->SetFlag(RELAY_PROTOCOL_FLAGS);
    m_pRelayProtocol->m_pRelayClient = this;
}

void NATTraversalRelayClient::CallRequestProbeInitiation(ProtocolCallContext* pCallContext,
                                                         const qList<StationURL>& rTargetList)
{
    m_pRelayProtocol->m_pProtocolClient->CallRequestProbeInitiation(pCallContext, rTargetList);
}

void NATTraversalRelayClient::CallRequestProbeInitiationExt(ProtocolCallContext* pCallContext,
                                                            const qList<StationURL>& rTargetList,
                                                            const StationURL& rStationToProbe)
{
    m_pRelayProtocol->m_pProtocolClient->CallRequestProbeInitiationExt(pCallContext, rTargetList, rStationToProbe);
}

void NATTraversalRelayClient::CallReportNATTraversalResult(ProtocolCallContext* pCallContext, const unsigned int& rCID,
                                                           const bool& rResult, const unsigned int& rRTT)
{
    m_pRelayProtocol->m_pProtocolClient->CallReportNATTraversalResult(pCallContext, rCID, rResult, rRTT);
}

void NATTraversalRelayClient::CallReportNATProperties(ProtocolCallContext* pCallContext,
                                                      const unsigned int& rNATMapping,
                                                      const unsigned int& rNATFiltering, const unsigned int& rRTT)
{
    m_pRelayProtocol->m_pProtocolClient->CallReportNATProperties(pCallContext, rNATMapping, rNATFiltering, rRTT);
}

bool NATTraversalRelayClient::CallGetRelaySignatureKey(ProtocolCallContext* pCallContext, int* pRelayMode,
                                                       DateTime* pCurrentUTCTime, String* pRelayAddress,
                                                       unsigned short* pRelayPort, int* pRelayAddressType,
                                                       unsigned int* pGameServerID)
{
    return m_pRelayProtocol->m_pProtocolClient->CallGetRelaySignatureKey(
        pCallContext, pRelayMode, pCurrentUTCTime, pRelayAddress, pRelayPort, pRelayAddressType, pGameServerID);
}

void NATTraversalRelayClient::CallReportNATTraversalResultDetail(ProtocolCallContext* pCallContext,
                                                                 const unsigned int& rCID, const bool& rResult,
                                                                 const NATTraversalResult& rResultDetail,
                                                                 unsigned int& rRTT)
{
    if (m_pRelayProtocol->m_pProtocolClient != 0)
    {
        Credentials* pCredentials = m_pRelayProtocol->m_pProtocolClient->m_pDefaultCredentials;
        if (pCredentials != 0 && pCredentials->m_secureConnection != 0
            && pCredentials->m_secureConnection->IsConnected())
        {
            const int iConnectionType = rResultDetail;
            m_pRelayProtocol->m_pProtocolClient->CallReportNATTraversalResultDetail(pCallContext, rCID, rResult,
                                                                                    iConnectionType, rRTT);
        }
    }
}

bool NATTraversalRelayClient::IsConnected() const
{
    EndPoint* pConnection = GetConnection(NAT_TRAVERSAL_CONNECTION_TYPE);
    return pConnection != 0 && pConnection->IsConnected();
}

NATTraversalRelayProtocol* NATTraversalRelayClient::CreateNATTraversalRelayProtocol()
{
    return new NATTraversalRelayProtocolImpl;
}

void NATTraversalRelayClient::UpdateProtocolsDefaultCredentials(Credentials* pCredentials)
{
    m_pRelayProtocol->SetDefaultCredentials(pCredentials);
    if (m_pCredentials != 0 && m_pCredentials->m_streamManager != 0)
        m_pCredentials->m_streamManager->GetProtocolRequestBroker()->UnregisterServerProtocol(
            NAT_TRAVERSAL_PROTOCOL_ID);
    if (pCredentials != 0 && pCredentials->m_streamManager != 0)
        pCredentials->m_streamManager->GetProtocolRequestBroker()->RegisterServerProtocol(NAT_TRAVERSAL_PROTOCOL_ID,
                                                                                          m_pRelayProtocol);
}

void NATTraversalRelayClient::ConnectionStateHasChanged()
{
    EndPoint* pConnection = GetConnection(NAT_TRAVERSAL_CONNECTION_TYPE);
    m_pRelayProtocol->SetClientConnection(pConnection);
    if (m_pRelayInterface != 0)
    {
        if (pConnection != 0)
        {
            m_pRelayInterface->UpdateConnectionState(m_pCredentials->m_state, pConnection->m_stationURL);
        }
        else
        {
            m_pRelayInterface->UpdateConnectionState(0, StationURL());
        }
    }
}

}
}
