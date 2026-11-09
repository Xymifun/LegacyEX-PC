#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBCAFELOGIN_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBCAFELOGIN_HPP

#include "nn/nex/JobLogin.hpp"
#include "nn/nex/ObjectThread.hpp"

namespace nn
{
namespace nex
{

class JobCafeLogin : public JobLogin
{
public:
    explicit JobCafeLogin(unsigned int uiPrincipalId = 0);
    virtual ~JobCafeLogin();

    virtual void StepFirst();

    void StepRegisterAccount();
    void StepWaitingForRegisterAccount();
    void StepGetGameAuthentication();
    void StepWaitingForGetGameAuthentication();
    void StepGameLogin();
    void StepWaitingForGameLogin();
    void NexAuthenticationFunc(void* pParam);

private:
    typedef ObjectThread<JobCafeLogin, void*> AuthenticationThread;

    AuthenticationThread* m_pAuthenticationThread;
    unsigned int m_uiPrincipalId;
};

}
}

#endif
