#include "nn/nex/HTTPURL.hpp"

#include <cwchar>

#include "nn/nex/HTTPStringUtilities.hpp"
#include "nn/nex/StringStream.hpp"

namespace nn
{
namespace nex
{

HTTPURL::HTTPURL(const String& rURL)
    : m_scheme(L"http"),
      m_username(L""),
      m_password(L""),
      m_host(L"localhost"),
      m_port(0),
      m_path(L""),
      m_parameters(L""),
      m_query(L""),
      m_fragment(L"")
{
    ParseURL(rURL);
}

HTTPURL::~HTTPURL()
{
}

String HTTPURL::ConvertToString() const
{
    String url;
    url += m_scheme + L"://";
    if (m_username.GetLength() > 0)
    {
        url += m_username;
        if (m_password.GetLength() > 0)
        {
            url += L":" + m_password;
        }
        url += L"@";
    }
    url += m_host;
    if (m_port != 0)
    {
        StringStream portStream;
        portStream.StreamNumber(m_port);
        url += L":";
        url += portStream.GetBuffer();
    }
    url += L"/" + m_path;
    if (m_parameters.GetLength() > 0)
    {
        url += L";" + m_parameters;
    }
    if (m_query.GetLength() > 0)
    {
        url += L"?" + m_query;
    }
    if (m_fragment.GetLength() > 0)
    {
        url += L"#" + m_fragment;
    }
    return url;
}

void HTTPURL::ParseURL(const String& rURL)
{
    String remaining(rURL);
    m_scheme = HTTPStringUtilities::GetNextToken(remaining, String(L"://"));
    if (m_scheme.GetLength() == 0)
    {
        m_scheme = L"http";
    }
    String authority = HTTPStringUtilities::GetNextToken(remaining, String(L"/"));
    String credentials = HTTPStringUtilities::GetNextToken(authority, String(L"@"));
    if (authority.GetLength() > 0)
    {
        m_username = HTTPStringUtilities::GetNextToken(credentials, String(L":"));
        m_password = credentials;
        m_host = HTTPStringUtilities::GetNextToken(authority, String(L":"));
    }
    else
    {
        authority = credentials;
        m_username = L"";
        m_password = L"";
        m_host = HTTPStringUtilities::GetNextToken(authority, String(L":"));
    }
    unsigned long uiPort = 0;
    if (authority.GetLength() > 0)
    {
        uiPort = std::wcstoul(authority, 0, 10);
    }
    m_port = uiPort;
    m_path = remaining;
    int iPosition = m_path.FindSubstringCase(L"#", 0);
    if (iPosition == -1)
    {
        m_fragment = L"";
        iPosition = m_path.FindSubstringCase(L"?", 0);
    }
    else
    {
        m_fragment = HTTPStringUtilities::TruncateSuffix(m_path, iPosition, 1);
        iPosition = m_path.FindSubstringCase(L"?", 0);
    }
    if (iPosition == -1)
    {
        m_query = L"";
        iPosition = m_path.FindSubstringCase(L";", 0);
    }
    else
    {
        m_query = HTTPStringUtilities::TruncateSuffix(m_path, iPosition, 1);
        iPosition = m_path.FindSubstringCase(L";", 0);
    }
    if (iPosition == -1)
    {
        m_parameters = L"";
    }
    else
    {
        m_parameters = HTTPStringUtilities::TruncateSuffix(m_path, iPosition, 1);
    }
}

void HTTPURL::SetLocation(const String& rHost, unsigned int uiPort)
{
    m_host = rHost;
    m_port = uiPort;
}

void HTTPURL::SetAuthentification(const String& rUsername, const String& rPassword)
{
    m_username = rUsername;
    if (m_username.GetLength() > 0)
    {
        m_password = rPassword;
    }
    else
    {
        m_password = L"";
    }
}

void HTTPURL::Trace(unsigned int) const
{
}

}
}
