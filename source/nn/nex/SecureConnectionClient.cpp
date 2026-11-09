#include "nn/nex/SecureConnectionClient.hpp"

#include "nn/nex/AnyDataAdapter.hpp"
#include "nn/nex/ClientProtocol.hpp"
#include "nn/nex/ConnectionManager.hpp"
#include "nn/nex/ConnectionOrientedStream.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/MemoryManager.hpp"
#include "nn/nex/NATProperties.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/StreamManager.hpp"
#include "nn/nex/StreamTable.hpp"
#include "nn/nex/StringStream.hpp"
#include "nn/nex/qBuffer.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int SECURE_CONNECTION_SERVICE_ID = 2;
const unsigned short SECURE_CONNECTION_PROTOCOL_ID = 11;
const NATProperties::MappingProperties NAT_MAPPING_UNKNOWN = static_cast<NATProperties::MappingProperties>(0);
const NATProperties::FilteringProperties NAT_FILTERING_UNKNOWN = static_cast<NATProperties::FilteringProperties>(0);
bool g_bRequestSuccess;

}

SecureConnectionClient::SecureConnectionClient() : ServiceClient(SECURE_CONNECTION_SERVICE_ID), m_protocolClient(0)
{
    AnyDataAdapter::Register();
    m_protocolClient.SetProtocolID(SECURE_CONNECTION_PROTOCOL_ID);
    SecureConnectionProtocolClient* pProtocolClient = new SecureConnectionProtocolClient();
    GetProtocolRegistry()->RegisterProtocol(SECURE_CONNECTION_PROTOCOL_ID, pProtocolClient);
}

SecureConnectionClient::~SecureConnectionClient()
{
    AnyDataAdapter::Unregister();
}

void SecureConnectionClient::PrepareURLs(qList<StationURL>& rURLs)
{
    Network::GetInstance()->GetLocalURLs(&rURLs);
    ConnectionOrientedStream* pStream = GetStreamManager()->GetConnectionManager()->GetStreamTable().FindStream(
        L"prudp");
    for (qList<StationURL>::iterator it = rURLs.begin(); it != rURLs.end(); ++it)
    {
        it->SetStreamID(pStream->GetStreamID());
        it->SetNATMapping(NAT_MAPPING_UNKNOWN);
        it->SetNATFiltering(NAT_FILTERING_UNKNOWN);
        it->SetUPnPSupport(false);
        it->SetNatPMPSupport(false);
        it->SetPlatformType();
        Network::GetInstance()->ReplaceLocalURL(*it);
    }
}

bool SecureConnectionClient::RegisterURLs(ProtocolCallContext* pCallContext, qResult* pResult, StationURL* pPublicURL)
{
    qList<StationURL> urls;
    PrepareURLs(urls);
    return m_protocolClient.CallRegister(pCallContext, pResult, urls, &m_pCredentials->m_state, pPublicURL);
}

bool SecureConnectionClient::RegisterURLsEx(ProtocolCallContext* pCallContext, qResult* pResult, StationURL* pPublicURL,
                                            const AnyObjectHolder<Data, String>& rCustomData)
{
    qList<StationURL> urls;
    PrepareURLs(urls);
    if (rCustomData.GetObject() == 0)
        return m_protocolClient.CallRegister(pCallContext, pResult, urls, &m_pCredentials->m_state, pPublicURL);

    return m_protocolClient.CallRegisterEx(pCallContext, pResult, urls, &m_pCredentials->m_state, pPublicURL,
                                           rCustomData);
}

bool SecureConnectionClient::RequestConnectionData(ProtocolCallContext* pCallContext, unsigned int uiConnectionID,
                                                   unsigned int uiPrincipalID, qList<ConnectionData>* pConnectionData)
{
    return m_protocolClient.CallRequestConnectionData(pCallContext, &g_bRequestSuccess, uiConnectionID, uiPrincipalID,
                                                      pConnectionData);
}

bool SecureConnectionClient::RequestURLs(ProtocolCallContext* pCallContext, unsigned int uiConnectionID,
                                         unsigned int uiPrincipalID, qList<StationURL>* pURLs)
{
    return m_protocolClient.CallRequestURLs(pCallContext, &g_bRequestSuccess, uiConnectionID, uiPrincipalID, pURLs);
}

bool SecureConnectionClient::RequestURLs(ProtocolCallContext* pCallContext, unsigned int uiConnectionID,
                                         qList<StationURL>* pURLs)
{
    return RequestURLs(pCallContext, uiConnectionID, 0, pURLs);
}

bool SecureConnectionClient::TestConnectivity(ProtocolCallContext* pCallContext)
{
    return m_protocolClient.CallTestConnectivity(pCallContext);
}

bool SecureConnectionClient::UpdateURLs(ProtocolCallContext* pCallContext)
{
    qList<StationURL> urls;
    Network::GetInstance()->GetLocalURLs(&urls);
    return m_protocolClient.CallUpdateURLs(pCallContext, urls);
}

bool SecureConnectionClient::ReplaceURL(ProtocolCallContext* pCallContext, const StationURL& rTarget,
                                        const StationURL& rReplacement)
{
    if (rTarget.GetPortNumber() == 0)
        return false;

    if (rReplacement.GetPortNumber() == 0)
        return false;

    if (!Network::GetInstance()->ReplaceLocalURL(rTarget, rReplacement))
        return false;

    return m_protocolClient.CallReplaceURL(pCallContext, rTarget, rReplacement);
}

void SecureConnectionClient::SendReport(unsigned int uiReportID, const void* pReport, unsigned int uiReportSize)
{
    if (pReport != 0 && uiReportSize != 0)
    {
        qBuffer report(0, 0);
        report.push_back(static_cast<const unsigned char*>(pReport), uiReportSize);
        StringStream stream;
        noshowbase(stream);
        zerohex(stream);
        for (unsigned int i = 0; i < report.size(); ++i)
            stream.StreamNumber(report.front()[i]);

        stream.GetLength();
        ProtocolCallContext callContext(0);
        m_protocolClient.CallSendReport(&callContext, uiReportID, report);
    }
}

void SecureConnectionClient::UpdateProtocolsDefaultCredentials(Credentials* pCredentials)
{
    m_protocolClient.SetDefaultCredentials(pCredentials);
}

}
}
