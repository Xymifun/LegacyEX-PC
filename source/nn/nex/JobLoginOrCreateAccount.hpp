#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBLOGINORCREATEACCOUNT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBLOGINORCREATEACCOUNT_HPP

#include "nn/nex/CallContext.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/SandboxConnectionInfo.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class AccountManagementCommand;
class BackEndServices;
class Credentials;
class Data;
class GuestCreateAccountCommand;

template <typename T, typename KeyType>
class AnyObjectHolder;

class JobLoginOrCreateAccount : public StepSequenceJob
{
public:
    JobLoginOrCreateAccount(unsigned int uiCallId, BackEndServices* pBackEndServices,
                            const SandboxConnectionInfo& rSandboxConnectionInfo, const String& rName,
                            const String& rPassword, const String& rEmail, unsigned int uiGroups,
                            Credentials** ppCredentials, String* pErrorMessage,
                            AnyObjectHolder<Data, String>* pExtraData);
    virtual ~JobLoginOrCreateAccount();

    virtual void CompleteJob(const qResult& rResult);

    void SetCreateAccountCommand(GuestCreateAccountCommand* pCommand);
    void RVLogin();
    void ProcessRVLoginResult();
    void CreateAccount();
    void ProcessCreateAccountResult();

private:
    unsigned int m_uiCallId;
    String m_name;
    String m_password;
    String m_email;
    unsigned int m_uiGroups;
    Credentials** m_ppCredentials;
    String* m_pErrorMessage;
    AnyObjectHolder<Data, String>* m_pExtraData;
    CallContext m_loginContext;
    ProtocolCallContext m_createContext;
    AccountManagementCommand* m_pCreateAccountCommand;
    bool m_bCreateAccountAttempted;
    SandboxConnectionInfo m_sandboxConnectionInfo;
    Time m_deadline;
    BackEndServices* m_pBackEndServices;
};

}
}

#endif
