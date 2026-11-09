#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NGSFACADE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NGSFACADE_HPP

#include "nn/nex/AuthenticationInfo.hpp"
#include "nn/nex/CustomDataHolder.hpp"
#include "nn/nex/NgsBridgeImp.hpp"
#include "nn/nex/RendezVous.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class Job;
class JobCafeLogin;
class JobLogin;
class JobNNIDLogin;
class ProtocolCallContext;
class UserContext;

class NgsFacade : public RendezVous
{
public:
    NgsFacade();
    virtual ~NgsFacade();

    bool TerminateJobIsInProgress() const;
    void TerminateImpl(CallContext* pCallContext);
    static void OnTerminate(CallContext* pCallContext, const UserContext* pUserContext);

    qResult LoginAndRequestAuthenticationTokenImpl(ProtocolCallContext* pCallContext, unsigned int uiLoginParam,
                                                   const wchar_t* pSandboxAccessKey, const char* pFirstString,
                                                   char* pSecondString, const void* pToken, int iFirstValue,
                                                   unsigned int uiSecondValue);
    qResult LoginAndRequestAuthenticationToken(ProtocolCallContext* pCallContext, unsigned int uiLoginParam,
                                               const wchar_t* pSandboxAccessKey, const char* pFirstString,
                                               char* pSecondString, int iFirstValue, unsigned int uiSecondValue);
    qResult Login(ProtocolCallContext* pCallContext, unsigned int uiLoginParam, const wchar_t* pSandboxAccessKey,
                  int iFirstValue, unsigned int uiSecondValue);
    qResult Login(ProtocolCallContext* pCallContext, const void* pToken, const wchar_t* pSandboxAccessKey,
                  int iFirstValue, unsigned int uiSecondValue);
    qResult Logout(ProtocolCallContext* pCallContext);
    bool LoginJobIsInProgress() const;
    int GetLastLoginErrorCode() const;

    int GetNasErrorCode() const { return m_iNasErrorCode; }

    int GetHttpStatusCode() const { return m_iHttpStatusCode; }

    NgsBridgeInterface* GetNgsBridge() { return m_pNgsBridge; }

private:
    friend class JobLogin;
    friend class JobCafeLogin;
    friend class JobNNIDLogin;

    unsigned int m_reserved0;
    Job* m_pLoginJob;
    Job* m_pTerminateJob;
    NgsBridgeImp* m_pNgsBridge;
    int m_iNasErrorCode;
    int m_iHttpStatusCode;
    qResult m_loginResult;
    unsigned int m_uiLoginErrorCode;
    bool m_reserved1;
    bool m_reserved2;
    bool m_reserved3;
    CustomDataHolder<AuthenticationInfo> m_authenticationInfo;
};

}
}

#endif
