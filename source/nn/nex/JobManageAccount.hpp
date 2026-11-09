#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBMANAGEACCOUNT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBMANAGEACCOUNT_HPP

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/CallContext.hpp"
#include "nn/nex/Data.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class AccountManagementCommand;
class AccountManagementProtocolClient;

class JobManageAccount : public StepSequenceJob
{
public:
    enum FuncType
    {
        CreateAccountFunc = 0,
        CreateAccountWithCustomDataFunc = 1,
        ChangePasswordFunc = 2,
        UpdateAccountFunc = 3,
        ChangePasswordByGuestFunc = 4,
        CustomCreateAccountFunc = 5,
        NintendoCreateAccountFunc = 6
    };

    JobManageAccount(unsigned int uiCallId, AccountManagementProtocolClient* pProtocolClient, const String& rName,
                     const String& rPassword, unsigned int uiGroups, const String& rEmail, qResult* pResult,
                     FuncType funcType, AnyObjectHolder<Data, String>* pPublicData,
                     AnyObjectHolder<Data, String>* pPrivateData, AnyObjectHolder<Data, String>* pCustomData,
                     unsigned int* pPrincipalID, String* pPidHmac);
    JobManageAccount(unsigned int uiCallId, AccountManagementProtocolClient* pProtocolClient,
                     AccountManagementCommand* pCommand);
    JobManageAccount(unsigned int uiCallId, AccountManagementProtocolClient* pProtocolClient, const String& rPassword,
                     bool* pSuccess, FuncType funcType);
    virtual ~JobManageAccount();

    void Cleanup();
    void Cancel();
    void CompleteJobImpl(CallContext::State state, qResult result);
    void PasswordInitialHash();
    void PasswordInitialHashCompletion();
    void CallCreateAccount();
    void CallCreateAccountWithCustomData();
    void CallChangePassword();
    void CallUpdateAccount();
    void CallChangePasswordByGuest();
    void CallCustomCreateAccount();
    void CallNintendoCreateAccount();
    void CallAccountManagementCommand();
    void CompleteJob();

private:
    unsigned int m_uiCallId;
    AccountManagementProtocolClient* m_pProtocolClient;
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
    Key m_key;
    qResult* m_pResult;
    bool* m_pSuccess;
    CallContext m_keyContext;
    ProtocolCallContext m_callContext;
    AccountManagementCommand* m_pCommand;
};

}
}

#endif
