#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPURL_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPURL_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class HTTPURL : public RootObject
{
public:
    explicit HTTPURL(const String& rURL);
    ~HTTPURL();

    String ConvertToString() const;
    void ParseURL(const String& rURL);
    void SetLocation(const String& rHost, unsigned int uiPort);
    void SetAuthentification(const String& rUsername, const String& rPassword);
    String StripLocation() const;

    const String& GetScheme() const { return m_scheme; }

    const String& GetUsername() const { return m_username; }

    const String& GetPassword() const { return m_password; }

    const String& GetHost() const { return m_host; }

    unsigned int GetPort() const { return m_port; }

    const String& GetPath() const { return m_path; }

    const String& GetParameters() const { return m_parameters; }

    const String& GetQuery() const { return m_query; }

    const String& GetFragment() const { return m_fragment; }

    void SetScheme(const String& rScheme);

    void SetPath(const String& rPath) { m_path = rPath; }

    void SetParameters(const String& rParameters) { m_parameters = rParameters; }

    void SetQuery(const String& rQuery) { m_query = rQuery; }

    void SetFragment(const String& rFragment) { m_fragment = rFragment; }

    static String Escape(const String& rValue);
    static String Unescape(const String& rValue);

    void Trace(unsigned int uiFilter) const;

private:
    String m_scheme;
    String m_username;
    String m_password;
    String m_host;
    unsigned int m_port;
    String m_path;
    String m_parameters;
    String m_query;
    String m_fragment;
};

}
}

#endif
