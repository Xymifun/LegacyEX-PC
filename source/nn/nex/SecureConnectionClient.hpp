#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SECURECONNECTIONCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SECURECONNECTIONCLIENT_HPP

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/SecureConnectionProtocolClient.hpp"
#include "nn/nex/ServiceClient.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class ConnectionData;
class Credentials;
class Data;
class ProtocolCallContext;
class StationURL;
class qResult;

class SecureConnectionClient : public ServiceClient
{
public:
    SecureConnectionClient();
    virtual ~SecureConnectionClient();

    virtual void UpdateProtocolsDefaultCredentials(Credentials* pCredentials);

    void PrepareURLs(qList<StationURL>& rURLs);
    bool RegisterURLs(ProtocolCallContext* pCallContext, qResult* pResult, StationURL* pPublicURL);
    bool RegisterURLsEx(ProtocolCallContext* pCallContext, qResult* pResult, StationURL* pPublicURL,
                        const AnyObjectHolder<Data, String>& rCustomData);
    bool RequestConnectionData(ProtocolCallContext* pCallContext, unsigned int uiConnectionID,
                               unsigned int uiPrincipalID, qList<ConnectionData>* pConnectionData);
    bool RequestURLs(ProtocolCallContext* pCallContext, unsigned int uiConnectionID, unsigned int uiPrincipalID,
                     qList<StationURL>* pURLs);
    bool RequestURLs(ProtocolCallContext* pCallContext, unsigned int uiConnectionID, qList<StationURL>* pURLs);
    bool TestConnectivity(ProtocolCallContext* pCallContext);
    bool UpdateURLs(ProtocolCallContext* pCallContext);
    bool ReplaceURL(ProtocolCallContext* pCallContext, const StationURL& rTarget, const StationURL& rReplacement);
    void SendReport(unsigned int uiReportID, const void* pReport, unsigned int uiReportSize);

private:
    SecureConnectionProtocolClient m_protocolClient;
};

}
}

#endif
