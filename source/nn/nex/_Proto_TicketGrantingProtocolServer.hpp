#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_TICKETGRANTINGPROTOCOLSERVER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_TICKETGRANTINGPROTOCOLSERVER_HPP

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/ServerProtocol.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class Data;
class RVConnectionData;

class _Proto_TicketGrantingProtocolServer : public ServerProtocol
{
public:
    enum _MethodID
    {
        METHOD_LOGIN = 1,
        METHOD_LOGIN_EX = 2,
        METHOD_REQUEST_TICKET = 3,
        METHOD_GET_PID = 4,
        METHOD_GET_NAME = 5,
        METHOD_LOGIN_WITH_CONTEXT = 6
    };

    explicit _Proto_TicketGrantingProtocolServer(unsigned int uiFlags) : ServerProtocol(uiFlags) {}

    virtual void DispatchProtocolMessage(Message* pRequest, Message* pResponse, bool* pResponseRequired,
                                         EndPoint* pEndPoint);
    virtual qResult Login(const String& rUserName, unsigned int* pPrincipalID, Buffer* pTicket,
                          RVConnectionData* pConnectionData, String* pReturnMessage) = 0;
    virtual qResult LoginEx(const String& rUserName, unsigned int* pPrincipalID, Buffer* pTicket,
                            RVConnectionData* pConnectionData, String* pReturnMessage,
                            const AnyObjectHolder<Data, String>& rCustomData) = 0;
    virtual qResult RequestTicket(const unsigned int& rSourcePrincipalID, const unsigned int& rTargetPrincipalID,
                                  Buffer* pTicket) = 0;
    virtual unsigned int GetPID(const String& rUserName) = 0;
    virtual String GetName(const unsigned int& rPrincipalID) = 0;
    virtual qResult LoginWithContext(const AnyObjectHolder<Data, String>& rLoginData, unsigned int* pPrincipalID,
                                     Buffer* pTicket, RVConnectionData* pConnectionData) = 0;
};

}
}

#endif
