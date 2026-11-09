#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBNNIDLOGIN_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBNNIDLOGIN_HPP

#include "nn/nex/JobLogin.hpp"

namespace nn
{
namespace nex
{

class JobNNIDLogin : public JobLogin
{
public:
    JobNNIDLogin();
    virtual ~JobNNIDLogin();

    void Initialize(unsigned int uiCallId, unsigned int uiLoginParam, NgsFacade* pNgsFacade, const char* pFirstString,
                    char* pSecondString, const void* pToken, int iTimeout, unsigned int uiPrincipalId);

    virtual void StepFirst();

    void StepGameLogin();
    void StepWaitingForGameLogin();

private:
    const ACTNexAuthenticationResult* m_pToken;
    unsigned int m_uiPrincipalId;
};

}
}

#endif
