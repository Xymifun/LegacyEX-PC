#ifndef SOURCE_NN_NEX_RENDEZVOUS_HPP
#define SOURCE_NN_NEX_RENDEZVOUS_HPP

#include "nn/nex/BackEndServices.hpp"

namespace nn
{
namespace nex
{

class DateTime;

class RendezVous : public BackEndServices
{
public:
    RendezVous();
    virtual ~RendezVous();

    bool Login(CallContext* pCallContext, const wchar_t* pUserName, const char* pPassword, const wchar_t* pHostName,
               unsigned short uiPort, const AnyObjectHolder<Data, String>& rExtraData, Credentials** ppCredentials,
               String* pErrorMessage);
    bool SilentLogin(CallContext* pCallContext, const wchar_t* pUserName, const char* pPassword,
                     const wchar_t* pHostName, unsigned short uiPort, Credentials** ppCredentials);
    bool Logout(CallContext* pCallContext, Credentials* pCredentials);
    bool SilentLogout(CallContext* pCallContext, Credentials* pCredentials);
    unsigned int GetGameServerTime(DateTime* pTime) const;
};

}
}

#endif
