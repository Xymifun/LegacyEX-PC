#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBLOGIN_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBLOGIN_HPP

#include "nn/nex/AuthenticationInfo.hpp"
#include "nn/nex/CustomDataHolder.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/qResult.hpp"

struct ACTNexAuthenticationResult
{
    enum
    {
        TOKEN_LENGTH = 516,
        PASSWORD_LENGTH = 68,
        HOST_LENGTH = 16,
        RESERVED_LENGTH = 2
    };

    char token[TOKEN_LENGTH];
    char password[PASSWORD_LENGTH];
    char host[HOST_LENGTH];
    unsigned short port;
    char reserved[RESERVED_LENGTH];
};

namespace nn
{
namespace nex
{

class Credentials;
class NgsFacade;

class JobLogin : public StepSequenceJob
{
public:
    JobLogin();
    virtual ~JobLogin();

    void Initialize(unsigned int uiCallId, unsigned int uiLoginParam, NgsFacade* pNgsFacade, const char* pFirstString,
                    char* pSecondString, int iTimeout);
    void Complete(const qResult& rResult);
    bool Wait();

    virtual void StepFirst() = 0;

protected:
    ProtocolCallContext m_protocolContext;
    unsigned int m_uiCallId;
    unsigned int m_uiLoginParam;
    NgsFacade* m_pNgsFacade;
    Credentials* m_pCredentials;
    const char* m_pFirstString;
    char* m_pSecondString;
    int m_iTimeout;
    Time m_startTime;
    CustomDataHolder<AuthenticationInfo> m_authenticationInfo;
    String m_errorMessage;
};

}
}

#endif
