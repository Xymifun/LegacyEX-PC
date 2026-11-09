#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TICKETMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TICKETMANAGER_HPP

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qMap.hpp"

namespace nn
{
namespace nex
{

class AuthenticationClient;
class CallContext;
class Data;
class LoginData;
class RVConnectionData;
class Ticket;
class qResult;

template <typename T>
class CustomDataHolder;

class TicketManager : public RootObject
{
public:
    explicit TicketManager(AuthenticationClient* pAuthenticationClient);
    ~TicketManager();

    bool Login(CallContext* pCallContext, qResult* pResult, const String& rUserName, const char* pPassword,
               unsigned int* pPrincipalID, RVConnectionData* pConnectionData, String* pReturnMessage,
               AnyObjectHolder<Data, String>* pCustomData);
    bool LoginWithData(CallContext* pCallContext, qResult* pResult, const char* pPassword, unsigned int* pPrincipalID,
                       RVConnectionData* pConnectionData, CustomDataHolder<LoginData>* pLoginData);
    bool AcquireTicket(CallContext* pCallContext, unsigned int uiSourcePrincipalID, unsigned int uiTargetPrincipalID,
                       Ticket** ppTicket);
    Ticket* FindTicket(unsigned int uiTargetPrincipalID);
    void InsertTicket(unsigned int uiTargetPrincipalID, Ticket* pTicket);
    void ReleaseTicket(Ticket* pTicket);

private:
    friend class JobTicketManagerAcquireTicket;
    friend class JobTicketManagerLogin;
    friend class JobTicketManagerLoginWithData;

    qMap<unsigned int, Ticket*> m_tickets;
    AuthenticationClient* m_pAuthenticationClient;
};

}
}

#endif
