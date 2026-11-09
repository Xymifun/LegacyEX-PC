#include "nn/nex/ServiceHttpClient.hpp"

#include <stdio.h>
#include <string.h>
#include <nn/act/client_cpp.h>

#include "nn/nex/StringStream.hpp"

namespace nn
{
namespace act
{

nn::Result GetHostServerSettings(char* pHost, char* pSettings,
                                 SlotNo slot) asm("GetHostServerSettings__Q2_2nn3actFPcT1Uc");

}
}

namespace nn
{
namespace nex
{

namespace
{

const unsigned int RESULT_ACT_NO_PRINCIPAL_ID = 0xA071F480;
const unsigned int PASSWORD_BUFFER_SIZE = 0x41;
const unsigned int HOST_BUFFER_SIZE = 40;
const unsigned int SETTINGS_BUFFER_SIZE = 4;
const unsigned int GAME_SERVER_ID_BUFFER_SIZE = 16;

const char* s_pHppDomain = "n.app.nintendo.net";
const char* s_pHppPath = "/hpp/";

}

ServiceHttpClient::~ServiceHttpClient()
{
}

bool ServiceHttpClient::InitializeNfsLibrary() const
{
    return nn::act::Initialize().IsSuccess();
}

bool ServiceHttpClient::GetPrincipalId(unsigned int& rPrincipalId) const
{
    unsigned int uiPrincipalId = 0;
    nn::Result result = nn::act::GetPrincipalIdEx(&uiPrincipalId, m_ucSlotNo);
    if (NNResult(result).value == static_cast<int>(RESULT_ACT_NO_PRINCIPAL_ID) || result.IsFailure())
        return false;
    rPrincipalId = uiPrincipalId;
    return true;
}

bool ServiceHttpClient::GetPassword(String& rPassword) const
{
    char acPassword[PASSWORD_BUFFER_SIZE] = {0};
    nn::Result result = nn::act::GetNfsPasswordEx(acPassword, m_ucSlotNo);
    if (result.IsFailure() || strncmp(acPassword, "", sizeof(acPassword)) == 0)
        return false;
    rPassword = acPassword;
    return true;
}

bool ServiceHttpClient::GetNfsSettings(String& rSettings) const
{
    char acHost[HOST_BUFFER_SIZE];
    char acSettings[SETTINGS_BUFFER_SIZE];
    nn::Result result = nn::act::GetHostServerSettings(acHost, acSettings, m_ucSlotNo);
    if (result.IsFailure())
        return false;
    rSettings = acSettings;
    return true;
}

void ServiceHttpClient::FinalizeNfsLibrary() const
{
    nn::act::Finalize();
}

String ServiceHttpClient::MakeHppUrl() const
{
    char acGameServerId[GAME_SERVER_ID_BUFFER_SIZE];
    snprintf(acGameServerId, GAME_SERVER_ID_BUFFER_SIZE - 1, "%08x", m_uiGameServerId);
    char* pSettings;
    m_nfsSettings.CreateCopy(&pSettings);
    String strSettings(pSettings);
    strSettings.ToLower();
    StringStream stream;
    stream << (m_bSecure ? L"https://" : L"http://") << L"hpp-" << String(acGameServerId) << L"-" << strSettings << L"."
           << String(s_pHppDomain) << String(s_pHppPath);
    String::ReleaseCopy(pSettings);
    return String(stream.GetBuffer());
}

bool ServiceHttpClient::SetSandboxContextImpl(unsigned int uiGameServerId, const wchar_t* pAccessKey)
{
    m_uiGameServerId = uiGameServerId;
    m_accessKey = pAccessKey;
    m_uiPrincipalId = 0;
    if (!InitializeNfsLibrary())
        return false;
    if (!GetPrincipalId(m_uiPrincipalId))
        return false;
    if (!GetPassword(m_password))
        return false;
    if (!GetNfsSettings(m_nfsSettings))
        return false;
    FinalizeNfsLibrary();
    m_hppUrl = MakeHppUrl();
    return true;
}

void ServiceHttpClient::SetSandboxContext(unsigned int uiGameServerId, const wchar_t* pAccessKey,
                                          unsigned char ucSlotNo)
{
    m_ucSlotNo = ucSlotNo;
    SetSandboxContextImpl(uiGameServerId, pAccessKey);
}

}
}
