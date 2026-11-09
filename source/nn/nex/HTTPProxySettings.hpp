#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPPROXYSETTINGS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPPROXYSETTINGS_HPP

#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class HTTPProxySettings
{
public:
    HTTPProxySettings();
    HTTPProxySettings(const String& proxyName, unsigned int proxyPort, const String& username, const String& password);
    ~HTTPProxySettings();

    const String& GetProxyName() const;
    unsigned int GetProxyPort() const;
    const String& GetUsername() const;
    const String& GetPassword() const;

    void SetProxyName(const String& proxyName);
    void SetProxyPort(unsigned int proxyPort);
    void SetUsername(const String& username);
    void SetPassword(const String& password);

    void Trace(unsigned int flags) const;

private:
    String m_proxyName;
    unsigned int m_proxyPort;
    String m_username;
    String m_password;
};

}
}

#endif
