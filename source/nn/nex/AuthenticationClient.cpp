#include "nn/nex/AuthenticationClient.hpp"

#include "nn/nex/AnyDataAdapter.hpp"
#include "nn/nex/CallContext.hpp"
#include "nn/nex/KerberosEncryption.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/TicketGrantingProtocolClient.hpp"
#include "nn/nex/TicketManager.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int AUTHENTICATION_SERVICE_ID = 1;
const unsigned short TICKET_GRANTING_PROTOCOL_ID = 10;
const unsigned int LOGIN_TIMEOUT = 30000;

}

AuthenticationClient::AuthenticationClient()
    : ServiceClient(AUTHENTICATION_SERVICE_ID),
      m_pTicketGrantingProtocolClient(0),
      m_pKerberosEncryption(0),
      m_pTicketManager(0),
      m_loginDeadline(0)
{
    m_pKerberosEncryption = new KerberosEncryption;
    m_pTicketManager = new TicketManager(this);
    m_pTicketGrantingProtocolClient = new TicketGrantingProtocolClient();
    m_pTicketGrantingProtocolClient->SetProtocolID(TICKET_GRANTING_PROTOCOL_ID);
    m_uiPrincipalID = 0;
    m_loginDeadline = Time::ConvertTimeoutToDeadline(LOGIN_TIMEOUT);
    AnyDataAdapter::Register();
    GetProtocolRegistry()->RegisterProtocol(TICKET_GRANTING_PROTOCOL_ID, new TicketGrantingProtocolClient());
}

AuthenticationClient::~AuthenticationClient()
{
    delete m_pKerberosEncryption;
    delete m_pTicketManager;
    delete m_pTicketGrantingProtocolClient;
    AnyDataAdapter::Unregister();
}

bool AuthenticationClient::Login(ProtocolCallContext* pCallContext, qResult* pResult, const String& rUserName,
                                 const char* pPassword, RVConnectionData* pConnectionData, String* pReturnMessage,
                                 AnyObjectHolder<Data, String>* pCustomData)
{
    m_loginDeadline = pCallContext->m_expirationTime;
    return m_pTicketManager->Login(pCallContext, pResult, rUserName, pPassword, &m_uiPrincipalID, pConnectionData,
                                   pReturnMessage, pCustomData);
}

bool AuthenticationClient::LoginWithData(ProtocolCallContext* pCallContext, qResult* pResult, const char* pPassword,
                                         RVConnectionData* pConnectionData, CustomDataHolder<LoginData>* pLoginData)
{
    m_loginDeadline = pCallContext->m_expirationTime;
    return m_pTicketManager->LoginWithData(pCallContext, pResult, pPassword, &m_uiPrincipalID, pConnectionData,
                                           pLoginData);
}

void AuthenticationClient::Logout()
{
    m_uiPrincipalID = 0;
}

bool AuthenticationClient::Rebind(Credentials* pCredentials)
{
    SetDefaultCredentials(pCredentials);
    return true;
}

void AuthenticationClient::UpdateProtocolsDefaultCredentials(Credentials* pCredentials)
{
    m_pTicketGrantingProtocolClient->SetDefaultCredentials(pCredentials);
}

bool AuthenticationClient::GetPID(ProtocolCallContext* pCallContext, unsigned int* pPrincipalID, String userName)
{
    return m_pTicketGrantingProtocolClient->CallGetPID(pCallContext, pPrincipalID, userName);
}

bool AuthenticationClient::GetName(ProtocolCallContext* pCallContext, String* pUserName, unsigned int uiPrincipalID)
{
    return m_pTicketGrantingProtocolClient->CallGetName(pCallContext, pUserName, uiPrincipalID);
}

}
}
