#include "nn/nex/RendezVous.hpp"

#include "nn/nex/RVClientCore.hpp"

namespace nn
{
namespace nex
{

RendezVous::RendezVous() : BackEndServices()
{
}

RendezVous::~RendezVous()
{
}

bool RendezVous::Login(CallContext* pCallContext, const wchar_t* pUserName, const char* pPassword,
                       const wchar_t* pHostName, unsigned short uiPort, const AnyObjectHolder<Data, String>& rExtraData,
                       Credentials** ppCredentials, String* pErrorMessage)
{
    OnLoginStart(*pCallContext);
    return LoginImpl(pCallContext, 0, pUserName, pPassword, pHostName, uiPort, ppCredentials, pErrorMessage,
                     const_cast<AnyObjectHolder<Data, String>*>(&rExtraData));
}

bool RendezVous::SilentLogin(CallContext* pCallContext, const wchar_t* pUserName, const char* pPassword,
                             const wchar_t* pHostName, unsigned short uiPort, Credentials** ppCredentials)
{
    return LoginImpl(pCallContext, 0, pUserName, pPassword, pHostName, uiPort, ppCredentials, 0, 0);
}

bool RendezVous::Logout(CallContext* pCallContext, Credentials* pCredentials)
{
    OnLogoutStart(*pCallContext);
    return Logout_(pCallContext, pCredentials);
}

bool RendezVous::SilentLogout(CallContext* pCallContext, Credentials* pCredentials)
{
    return Logout_(pCallContext, pCredentials);
}

unsigned int RendezVous::GetGameServerTime(DateTime* pTime) const
{
    return RVClientCore::GetInstance()->GetGameServerTime(pTime);
}

}
}
