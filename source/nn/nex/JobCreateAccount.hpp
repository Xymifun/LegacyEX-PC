#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBCREATEACCOUNT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBCREATEACCOUNT_HPP

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/Data.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class AccountManagementClient;
class Credentials;
class RendezVous;

class JobCreateAccount : public StepSequenceJob
{
public:
    enum FuncType
    {
        CreateAccountFunc = 0,
        CreateAccountWithCustomDataFunc = 1,
        ChangePasswordByGuestFunc = 2,
        CustomCreateAccountFunc = 3,
        NintendoCreateAccountFunc = 4
    };

    JobCreateAccount(unsigned int uiCallId, const char* pGuestPassword, const String& rHost, unsigned short uiPort,
                     const String& rName, const char* pPassword, unsigned int uiGroups, const String& rEmail,
                     FuncType funcType, AnyObjectHolder<Data, String>* pPublicData,
                     AnyObjectHolder<Data, String>* pPrivateData, AnyObjectHolder<Data, String>* pCustomData,
                     unsigned int* pPrincipalID, String* pPidHmac);
    virtual ~JobCreateAccount();

    void CompleteJob(qResult result);
    void LoginGuest();
    void ProcessLoginGuestResult();
    void CreateAccount();
    void CustomCreateAccount();
    void CreateAccountWithCustomData();
    void NintendoCreateAccount();
    void ChangePasswordByGuest();
    void ProcessCreateAccountResult();
    void LogoutGuest();
    void ProcessLogoutGuestResult();

private:
    unsigned int m_uiCallId;
    Credentials* m_pGuestCredentials;
    char* m_pGuestPassword;
    String m_host;
    unsigned short m_uiPort;
    String m_name;
    char* m_pPassword;
    unsigned int m_uiGroups;
    String m_email;
    FuncType m_funcType;
    AnyObjectHolder<Data, String> m_publicData;
    AnyObjectHolder<Data, String> m_privateData;
    AnyObjectHolder<Data, String> m_customData;
    unsigned int* m_pPrincipalID;
    String* m_pPidHmac;
    qResult m_createResult;
    qResult m_jobResult;
    RendezVous* m_pRendezVous;
    AccountManagementClient* m_pAccountManagementClient;
    ProtocolCallContext m_loginContext;
    ProtocolCallContext m_createContext;
    ProtocolCallContext m_logoutContext;
    ProtocolCallContext m_reservedContext;
};

}
}

#endif
