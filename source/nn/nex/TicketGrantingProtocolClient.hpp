#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TICKETGRANTINGPROTOCOLCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TICKETGRANTINGPROTOCOLCLIENT_HPP

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/ClientProtocol.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class Data;
class Message;
class ProtocolCallContext;
class RVConnectionData;
class qResult;

class TicketGrantingProtocolClient : public ClientProtocol
{
public:
    explicit TicketGrantingProtocolClient(unsigned short uiProtocolID = 0);

    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext);
    virtual Protocol* CreateResponder() const;

    static void ProtoReturn_Login(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallLogin(ProtocolCallContext* pCallContext, qResult* pResult, const String& rUserName,
                   unsigned int* pPrincipalID, Buffer* pTicket, RVConnectionData* pConnectionData,
                   String* pReturnMessage);
    static void ProtoReturn_LoginEx(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallLoginEx(ProtocolCallContext* pCallContext, qResult* pResult, const String& rUserName,
                     unsigned int* pPrincipalID, Buffer* pTicket, RVConnectionData* pConnectionData,
                     String* pReturnMessage, const AnyObjectHolder<Data, String>& rCustomData);
    static void ProtoReturn_RequestTicket(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallRequestTicket(ProtocolCallContext* pCallContext, qResult* pResult, const unsigned int& rSourcePrincipalID,
                           const unsigned int& rTargetPrincipalID, Buffer* pTicket);
    static void ProtoReturn_GetPID(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetPID(ProtocolCallContext* pCallContext, unsigned int* pPrincipalID, const String& rUserName);
    static void ProtoReturn_GetName(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetName(ProtocolCallContext* pCallContext, String* pUserName, const unsigned int& rPrincipalID);
    static void ProtoReturn_LoginWithContext(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallLoginWithContext(ProtocolCallContext* pCallContext, qResult* pResult,
                              const AnyObjectHolder<Data, String>& rLoginData, unsigned int* pPrincipalID,
                              Buffer* pTicket, RVConnectionData* pConnectionData);

    static bool Login_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, qResult* pResult,
                                     const String& rUserName, unsigned int* pPrincipalID, Buffer* pTicket,
                                     RVConnectionData* pConnectionData, String* pReturnMessage);
    static bool LoginEx_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, qResult* pResult,
                                       const String& rUserName, unsigned int* pPrincipalID, Buffer* pTicket,
                                       RVConnectionData* pConnectionData, String* pReturnMessage,
                                       const AnyObjectHolder<Data, String>& rCustomData);
    static bool RequestTicket_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, qResult* pResult,
                                             const unsigned int& rSourcePrincipalID,
                                             const unsigned int& rTargetPrincipalID, Buffer* pTicket);
    static bool GetPID_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, unsigned int* pPrincipalID,
                                      const String& rUserName);
    static bool GetName_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, String* pUserName,
                                       const unsigned int& rPrincipalID);
    static bool LoginWithContext_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, qResult* pResult,
                                                const AnyObjectHolder<Data, String>& rLoginData,
                                                unsigned int* pPrincipalID, Buffer* pTicket,
                                                RVConnectionData* pConnectionData);
};

}
}

#endif
