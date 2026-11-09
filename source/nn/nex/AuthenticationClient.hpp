#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_AUTHENTICATIONCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_AUTHENTICATIONCLIENT_HPP

#include <stdint.h>

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/ServiceClient.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class Credentials;
class Data;
class KerberosEncryption;
class LoginData;
class ProtocolCallContext;
class RVConnectionData;
class TicketGrantingProtocolClient;
class TicketManager;
class qResult;

template <typename T>
class CustomDataHolder;

class AuthenticationClient : public ServiceClient
{
public:
    AuthenticationClient();
    virtual ~AuthenticationClient();

    virtual void UpdateProtocolsDefaultCredentials(Credentials* pCredentials);

    bool Login(ProtocolCallContext* pCallContext, qResult* pResult, const String& rUserName, const char* pPassword,
               RVConnectionData* pConnectionData, String* pReturnMessage, AnyObjectHolder<Data, String>* pCustomData);
    bool LoginWithData(ProtocolCallContext* pCallContext, qResult* pResult, const char* pPassword,
                       RVConnectionData* pConnectionData, CustomDataHolder<LoginData>* pLoginData);
    void Logout();
    bool Rebind(Credentials* pCredentials);
    bool GetPID(ProtocolCallContext* pCallContext, unsigned int* pPrincipalID, String userName);
    bool GetName(ProtocolCallContext* pCallContext, String* pUserName, unsigned int uiPrincipalID);

private:
    friend class JobBackEndServicesLogin;
    friend class JobBackEndServicesLoginWithData;
    friend class JobConnectSecureEndPoint;
    friend class JobTicketManagerAcquireTicket;
    friend class JobTicketManagerLogin;
    friend class JobTicketManagerLoginWithData;
    friend class KerberosAuthentication;
    friend class TicketManager;

    uint32_t m_unknown10;
    TicketGrantingProtocolClient* m_pTicketGrantingProtocolClient;
    KerberosEncryption* m_pKerberosEncryption;
    TicketManager* m_pTicketManager;
    unsigned int m_uiPrincipalID;
    Time m_loginDeadline;
};

}
}

#endif
