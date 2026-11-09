#include "nn/nex/HTTPProxySettings.hpp"

#include "nn/nex/HTTPURL.hpp"

namespace nn
{
namespace nex
{

HTTPProxySettings::HTTPProxySettings() : m_proxyName(L""), m_proxyPort(0), m_username(L""), m_password(L"")
{
}

HTTPProxySettings::HTTPProxySettings(const String& proxyName, unsigned int proxyPort, const String& username,
                                     const String& password)
    : m_proxyName(proxyName),
      m_proxyPort(proxyPort),
      m_username(username),
      m_password(password)
{
}

HTTPProxySettings::~HTTPProxySettings()
{
}

const String& HTTPProxySettings::GetProxyName() const
{
    return m_proxyName;
}

unsigned int HTTPProxySettings::GetProxyPort() const
{
    return m_proxyPort;
}

const String& HTTPProxySettings::GetUsername() const
{
    return m_username;
}

const String& HTTPProxySettings::GetPassword() const
{
    return m_password;
}

void HTTPProxySettings::Trace(unsigned int uiFilter) const
{
    HTTPURL url(String(L"http:///"));
    url.SetLocation(m_proxyName, m_proxyPort);
    if (m_username.GetLength() > 0)
    {
        url.SetAuthentification(m_username, m_password);
    }
    url.Trace(uiFilter);
}

}
}
