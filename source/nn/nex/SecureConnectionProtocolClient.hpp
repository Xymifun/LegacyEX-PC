#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SECURECONNECTIONPROTOCOLCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SECURECONNECTIONPROTOCOLCLIENT_HPP

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/ClientProtocol.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class ConnectionData;
class Data;
class Message;
class ProtocolCallContext;
class StationURL;
class qBuffer;
class qResult;

class SecureConnectionProtocolClient : public ClientProtocol
{
public:
    explicit SecureConnectionProtocolClient(unsigned short uiProtocolID = 0);

    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext);
    virtual Protocol* CreateResponder() const;

    static void ProtoReturn_Register(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallRegister(ProtocolCallContext* pCallContext, qResult* pResult, const qList<StationURL>& rURLs,
                      unsigned int* pConnectionID, StationURL* pPublicURL);
    static void ProtoReturn_RequestConnectionData(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallRequestConnectionData(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rConnectionID,
                                   const unsigned int& rPrincipalID, qList<ConnectionData>* pConnectionData);
    static void ProtoReturn_RequestURLs(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallRequestURLs(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rConnectionID,
                         const unsigned int& rPrincipalID, qList<StationURL>* pURLs);
    static void ProtoReturn_RegisterEx(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallRegisterEx(ProtocolCallContext* pCallContext, qResult* pResult, const qList<StationURL>& rURLs,
                        unsigned int* pConnectionID, StationURL* pPublicURL,
                        const AnyObjectHolder<Data, String>& rCustomData);
    static void ProtoReturn_TestConnectivity(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallTestConnectivity(ProtocolCallContext* pCallContext);
    static void ProtoReturn_UpdateURLs(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallUpdateURLs(ProtocolCallContext* pCallContext, const qList<StationURL>& rURLs);
    static void ProtoReturn_ReplaceURL(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallReplaceURL(ProtocolCallContext* pCallContext, const StationURL& rTarget, const StationURL& rReplacement);
    static void ProtoReturn_SendReport(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallSendReport(ProtocolCallContext* pCallContext, const unsigned int& rReportID, const qBuffer& rReport);

    static bool Register_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, qResult* pResult,
                                        const qList<StationURL>& rURLs, unsigned int* pConnectionID,
                                        StationURL* pPublicURL);
    static bool RequestConnectionData_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                     bool* pSuccess, const unsigned int& rConnectionID,
                                                     const unsigned int& rPrincipalID,
                                                     qList<ConnectionData>* pConnectionData);
    static bool RequestURLs_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                           const unsigned int& rConnectionID, const unsigned int& rPrincipalID,
                                           qList<StationURL>* pURLs);
    static bool RegisterEx_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, qResult* pResult,
                                          const qList<StationURL>& rURLs, unsigned int* pConnectionID,
                                          StationURL* pPublicURL, const AnyObjectHolder<Data, String>& rCustomData);
    static bool TestConnectivity_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage);
    static bool UpdateURLs_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                          const qList<StationURL>& rURLs);
    static bool ReplaceURL_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                          const StationURL& rTarget, const StationURL& rReplacement);
    static bool SendReport_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                          const unsigned int& rReportID, const qBuffer& rReport);
};

}
}

#endif
