#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SERVICEHTTPCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SERVICEHTTPCLIENT_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Credentials;

class ServiceHttpClient : public RootObject
{
public:
    enum PriorityType
    {
        PRIORITY_TYPE_DEFAULT = 0
    };

    ServiceHttpClient();
    virtual ~ServiceHttpClient();

    bool InitializeNfsLibrary() const;
    bool GetPrincipalId(unsigned int& rPrincipalId) const;
    bool GetPassword(String& rPassword) const;
    bool GetNfsSettings(String& rSettings) const;
    void FinalizeNfsLibrary() const;
    String MakeHppUrl() const;
    bool SetSandboxContextImpl(unsigned int uiGameServerId, const wchar_t* pAccessKey);
    void SetSandboxContext(unsigned int uiGameServerId, const wchar_t* pAccessKey, unsigned char ucSlotNo);

    Credentials* GetCredentials() const { return 0; }

protected:
    unsigned int m_uiGameServerId;
    String m_accessKey;
    unsigned int m_uiPrincipalId;
    String m_password;
    unsigned int m_uiUnknown18;
    String m_hppUrl;
    bool m_bSecure;
    int m_iTimeout;
    unsigned char m_ucSlotNo;
    String m_nfsSettings;
    PriorityType m_priorityType;
};

}
}

#endif
