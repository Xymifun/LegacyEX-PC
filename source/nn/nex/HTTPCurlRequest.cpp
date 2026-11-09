#include "nn/nex/HTTPCurlRequest.hpp"

#include <curl/curl.h>
#include <string.h>

#include "nn/nex/SpecialArray.hpp"
#include "nn/nex/StringConversion.hpp"

extern "C"
{
CURL* mw_curl_easy_init(void);
}

namespace nn
{
namespace nex
{

namespace
{

bool CheckCurlError(unsigned int uiError)
{
    return uiError == CURLE_OK;
}

unsigned int CurlHeaderWriter(char* pBuffer, unsigned int uiSize, unsigned int uiCount, void* pUserData)
{
    unsigned int uiLength = uiSize * uiCount;
    if (uiLength > 2)
    {
        char* pLine = SpecialNewArray<char>(uiLength - 1, L"", 0);
        memcpy(pLine, pBuffer, uiLength - 2);
        pLine[uiLength - 2] = 0;
        String header(pLine);
        SpecialDeleteArray(pLine);
        static_cast<qVector<String>*>(pUserData)->push_back(header);
    }
    return uiLength;
}

}

HTTPCurlRequest::HTTPCurlRequest() : m_pHandle(0), m_uiError(0), m_pHeaderList(0), m_bUnresolved32(false)
{
    m_pHandle = mw_curl_easy_init();
}

HTTPCurlRequest::~HTTPCurlRequest()
{
    void* pShare = 0;
    SetOption(CURLOPT_SHARE, pShare);
    Clear();
    curl_easy_cleanup(m_pHandle);
    m_pHandle = 0;
}

void HTTPCurlRequest::ClearError()
{
    m_uiError = 0;
}

void HTTPCurlRequest::Clear()
{
    ClearError();
    void* pHeaders = 0;
    SetOption(CURLOPT_HTTPHEADER, pHeaders);
    if (m_pHeaderList != 0)
    {
        curl_slist_free_all(static_cast<curl_slist*>(m_pHeaderList));
        m_pHeaderList = 0;
    }
    m_headers.clear();
}

bool HTTPCurlRequest::SetHeaderWriter(TransferCallback pCallback, void* pUserData)
{
    bool bResult = false;
    if (SetOption(CURLOPT_HEADERFUNCTION, pCallback) && SetOption(CURLOPT_HEADERDATA, pUserData))
    {
        bResult = true;
    }
    return bResult;
}

bool HTTPCurlRequest::SetMethod(HTTPMethod method)
{
    unsigned int uiOption = 0;
    if (static_cast<unsigned int>(method) <= HTTP_METHOD_PUT)
    {
        static const unsigned char s_methodOptions[3] = {CURLOPT_HTTPGET, CURLOPT_POST, CURLOPT_UPLOAD};
        uiOption = s_methodOptions[method];
    }
    int iEnabled = 1;
    return SetOption(uiOption, iEnabled);
}

bool HTTPCurlRequest::SetOption(unsigned int uiOption, const String& rValue)
{
    char* pValue;
    StringConversion::T2Char8Alloc(rValue, &pValue);
    bool bResult = SetOption(uiOption, pValue);
    StringConversion::T2Char8Release(pValue);
    return bResult;
}

bool HTTPCurlRequest::SetUrl(const HTTPURL& rURL)
{
    return SetOption(CURLOPT_URL, rURL.ConvertToString());
}

bool HTTPCurlRequest::Reset(void* pShare, HTTPMethod method, const HTTPURL& rURL)
{
    bool bResult = false;
    m_bUnresolved32 = false;
    Clear();
    curl_easy_reset(m_pHandle);
    SetOption(CURLOPT_SHARE, pShare);
    int iNoSignal = 1;
    SetOption(CURLOPT_NOSIGNAL, iNoSignal);
    SetHeaderWriter(CurlHeaderWriter, &m_headers);
    if (SetMethod(method) && SetUrl(rURL))
    {
        bResult = true;
    }
    return bResult;
}

bool HTTPCurlRequest::SetWriter(TransferCallback pCallback, void* pUserData)
{
    bool bResult = false;
    if (SetOption(CURLOPT_WRITEFUNCTION, pCallback) && SetOption(CURLOPT_WRITEDATA, pUserData))
    {
        bResult = true;
    }
    return bResult;
}

bool HTTPCurlRequest::SetReader(TransferCallback pCallback, void* pUserData, unsigned long long uiLength)
{
    bool bResult = false;
    long long iLength = static_cast<long long>(uiLength);
    if (SetOption(CURLOPT_READFUNCTION, pCallback) && SetOption(CURLOPT_READDATA, pUserData)
        && SetOption(CURLOPT_POSTFIELDSIZE_LARGE, iLength) && SetOption(CURLOPT_INFILESIZE, iLength))
    {
        bResult = true;
    }
    return bResult;
}

bool HTTPCurlRequest::AllowRedirection(bool bAllow)
{
    const unsigned int* pAllow;
    unsigned int uiAllow = bAllow;
    pAllow = &uiAllow;
    return SetOption(CURLOPT_FOLLOWLOCATION, pAllow);
}

bool HTTPCurlRequest::SetHeaders(const qVector<String>& rHeaders)
{
    for (qVector<String>::const_iterator it = rHeaders.begin(); it != rHeaders.end(); ++it)
    {
        char* pHeader = 0;
        StringConversion::T2Char8Alloc(*it, &pHeader);
        m_pHeaderList = curl_slist_append(static_cast<curl_slist*>(m_pHeaderList), pHeader);
        StringConversion::T2Char8Release(pHeader);
    }
    return SetOption(CURLOPT_HTTPHEADER, m_pHeaderList);
}

bool HTTPCurlRequest::SetProxy(const HTTPProxySettings& rProxy)
{
    String proxy;
    const String& rName = rProxy.GetProxyName();
    const String& rUsername = rProxy.GetUsername();
    const String& rPassword = rProxy.GetPassword();
    if (rName.GetLength() != 0)
    {
        if (rUsername.GetLength() != 0)
        {
            if (rPassword.GetLength() != 0)
            {
                proxy.Format(L"%s:%s@%s:%d", static_cast<const wchar_t*>(rUsername),
                             static_cast<const wchar_t*>(rPassword), static_cast<const wchar_t*>(rName),
                             rProxy.GetProxyPort());
            }
            else
            {
                proxy.Format(L"%s@%s:%d", static_cast<const wchar_t*>(rUsername), static_cast<const wchar_t*>(rName),
                             rProxy.GetProxyPort());
            }
        }
        else
        {
            proxy.Format(L"%s:%d", static_cast<const wchar_t*>(rName), rProxy.GetProxyPort());
        }
    }
    return SetOption(CURLOPT_PROXY, proxy);
}

bool HTTPCurlRequest::SetTransferBufferSize(unsigned int uiSize)
{
    return SetOption(CURLOPT_BUFFERSIZE, uiSize);
}

unsigned int HTTPCurlRequest::GetResponseCode()
{
    unsigned int uiResponseCode;
    GetInfo(CURLINFO_RESPONSE_CODE, uiResponseCode);
    return uiResponseCode;
}

const qVector<String>& HTTPCurlRequest::GetHeaders() const
{
    return m_headers;
}

void HTTPCurlRequest::Trace(unsigned int) const
{
}

void* HTTPCurlRequest::GetHandle()
{
    return m_pHandle;
}

template <typename T>
bool HTTPCurlRequest::SetOption(unsigned int uiOption, const T& rValue)
{
    m_uiError = curl_easy_setopt(m_pHandle, static_cast<CURLoption>(uiOption), rValue);
    return CheckCurlError(m_uiError);
}

template <typename T>
bool HTTPCurlRequest::GetInfo(unsigned int uiInfo, T& rValue)
{
    m_uiError = curl_easy_getinfo(m_pHandle, static_cast<CURLINFO>(uiInfo), &rValue);
    return CheckCurlError(m_uiError);
}

}
}
