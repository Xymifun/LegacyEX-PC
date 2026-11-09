#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SANDBOXCONNECTIONINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SANDBOXCONNECTIONINFO_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class SandboxConnectionInfo : public RootObject
{
public:
    class HostCredentials
    {
    public:
        HostCredentials();
        HostCredentials(const String& rHost, unsigned short uiPort, const String& rUserName, const String& rPassword);

        bool IsValid() const;
        void Trace(uint64_t uiFlag) const;

        String m_host;
        unsigned short m_uiPort;
        String m_userName;
        String m_password;
    };

    SandboxConnectionInfo();
    SandboxConnectionInfo(const String& rHost, unsigned short uiPort, const String& rUserName, const String& rPassword);
    SandboxConnectionInfo(const HostCredentials& rCredentials);
    SandboxConnectionInfo(const HostCredentials& rCredentialsDev, const HostCredentials& rCredentialsCert,
                          const HostCredentials& rCredentialsProd);
    ~SandboxConnectionInfo();

    const HostCredentials& GetCredentialsDev() const;
    void SetCredentialsDev(const HostCredentials& rCredentials);
    const HostCredentials& GetCredentialsCert() const;
    void SetCredentialsCert(const HostCredentials& rCredentials);
    const HostCredentials& GetCredentialsProd() const;
    void SetCredentialsProd(const HostCredentials& rCredentials);
    void SetCredentialsProd(const HostCredentials& rPrimaryCredentials, const HostCredentials& rSecondaryCredentials);

    void Trace(uint64_t uiFlag) const;

    static void CheckSettings(const HostCredentials& rCredentials, const String& rName);
    static unsigned int GetIssuerID();

private:
    HostCredentials m_credentialsDev;
    HostCredentials m_credentialsCert;
    HostCredentials m_credentialsProd;
    uint32_t m_unknown58;
};

}
}

#endif
