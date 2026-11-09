#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_SECURECONNECTIONPROTOCOLSERVER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_SECURECONNECTIONPROTOCOLSERVER_HPP

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/ServerProtocol.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class ConnectionData;
class Data;
class StationURL;
class qBuffer;

class _Proto_SecureConnectionProtocolServer : public ServerProtocol
{
public:
    enum _MethodID
    {
        METHOD_REGISTER = 1,
        METHOD_REQUEST_CONNECTION_DATA = 2,
        METHOD_REQUEST_URLS = 3,
        METHOD_REGISTER_EX = 4,
        METHOD_TEST_CONNECTIVITY = 5,
        METHOD_UPDATE_URLS = 6,
        METHOD_REPLACE_URL = 7,
        METHOD_SEND_REPORT = 8
    };

    explicit _Proto_SecureConnectionProtocolServer(unsigned int uiFlags) : ServerProtocol(uiFlags) {}

    virtual ~_Proto_SecureConnectionProtocolServer();

    virtual void DispatchProtocolMessage(Message* pRequest, Message* pResponse, bool* pResponseRequired,
                                         EndPoint* pEndPoint);
    virtual qResult Register(const qList<StationURL>& rURLs, unsigned int* pConnectionID, StationURL* pPublicURL) = 0;
    virtual bool RequestConnectionData(const unsigned int& rConnectionID, const unsigned int& rPrincipalID,
                                       qList<ConnectionData>* pConnectionData) = 0;
    virtual bool RequestURLs(const unsigned int& rConnectionID, const unsigned int& rPrincipalID,
                             qList<StationURL>* pURLs) = 0;
    virtual qResult RegisterEx(const qList<StationURL>& rURLs, unsigned int* pConnectionID, StationURL* pPublicURL,
                               const AnyObjectHolder<Data, String>& rCustomData) = 0;
    virtual void TestConnectivity() = 0;
    virtual void UpdateURLs(const qList<StationURL>& rURLs) = 0;
    virtual void ReplaceURL(const StationURL& rTarget, const StationURL& rReplacement) = 0;
    virtual void SendReport(const unsigned int& rReportID, const qBuffer& rReport) = 0;

private:
    enum
    {
        RESULT_NOT_IMPLEMENTED = static_cast<int>(0x80010002u),
        RESPONSE_METHOD_FLAG = 0x8000
    };
};


}
}

#endif
