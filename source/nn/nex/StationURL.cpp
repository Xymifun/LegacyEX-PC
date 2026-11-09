#include "nn/nex/StationURL.hpp"

#include <cstring>
#include <cwchar>

#include "nn/nex/SpecialArray.hpp"

namespace nn
{
namespace nex
{

namespace
{

const int URL_RESULT_SUCCESS = 0x00010001;
const int URL_RESULT_INVALID_ARGUMENT = 0x8001000A;
const int URL_RESULT_INVALID_POINTER = 0x8001000C;
const int URL_RESULT_BUFFER_TOO_SMALL = 0x8001000F;
const int URL_RESULT_INVALID_URL = 0x80050003;

const unsigned int URL_BUFFER_LENGTH = 1024;
const unsigned int URL_TYPE_BUFFER_LENGTH = 64;
const unsigned int ADDRESS_BUFFER_LENGTH = 128;

const wchar_t* const PARAM_ADDRESS = L"address";
const wchar_t* const PARAM_PORT = L"port";
const wchar_t* const PARAM_STREAM_ID = L"sid";
const wchar_t* const PARAM_STREAM_TYPE = L"stream";
const wchar_t* const PARAM_PRINCIPAL_ID = L"PID";
const wchar_t* const PARAM_CONNECTION_ID = L"CID";
const wchar_t* const PARAM_RV_CONNECTION_ID = L"RVCID";
const wchar_t* const PARAM_PROBE_REQUEST_ID = L"PRID";
const wchar_t* const PARAM_FAST_PROBE_RESPONSE = L"fastproberesponse";
const wchar_t* const PARAM_NAT_MAPPING = L"natm";
const wchar_t* const PARAM_NAT_FILTERING = L"natf";
const wchar_t* const PARAM_PROBE_REQUEST_INITIATION = L"probeinit";
const wchar_t* const PARAM_UPNP_SUPPORT = L"upnp";
const wchar_t* const PARAM_NAT_PMP_SUPPORT = L"pmp";
const wchar_t* const PARAM_TYPE = L"type";
const wchar_t* const PARAM_RELAY_SERVER_ADDRESS = L"Rsa";
const wchar_t* const PARAM_RELAY_SERVER_PORT = L"Rsp";
const wchar_t* const PARAM_USE_RELAY_SERVER = L"R";
const wchar_t* const PARAM_RELAY_ADDRESS = L"Ra";
const wchar_t* const PARAM_RELAY_PORT = L"Rp";
const wchar_t* const PARAM_PLATFORM_TYPE = L"Pl";

const unsigned int PLATFORM_TYPE = 2;

}

StationURL::StationURL() : m_pURL(0), m_bDataValid(true), m_bStrValid(false), m_bValid(true)
{
    m_pAddress = new InetAddress;
    SetURLType(Prudp);
}

StationURL::StationURL(const StationURL& rStationURL)
    : m_pURL(0),
      m_bDataValid(true),
      m_bStrValid(false),
      m_bValid(false)
{
    m_pAddress = new InetAddress;
    Copy(rStationURL);
}

StationURL::StationURL(const String& rURL) : m_pURL(0), m_bDataValid(false), m_bStrValid(true), m_bValid(false)
{
    m_pAddress = new InetAddress;
    SetURL(rURL);
}

StationURL::StationURL(const wchar_t* pURL) : m_pURL(0), m_bDataValid(false), m_bStrValid(true), m_bValid(false)
{
    m_pAddress = new InetAddress;
    SetURL(pURL);
}

StationURL::~StationURL()
{
    SetURL(0);
    delete m_pAddress;
}

StationURL& StationURL::operator=(const StationURL& rStationURL)
{
    Copy(rStationURL);
    return *this;
}

StationURL& StationURL::operator=(const String& rURL)
{
    SetURL(rURL);
    return *this;
}

bool StationURL::operator==(const StationURL& rStationURL) const
{
    return std::wcscmp(GetURL(), rStationURL.GetURL()) == 0;
}

bool StationURL::operator!=(const StationURL& rStationURL) const
{
    return std::wcscmp(GetURL(), rStationURL.GetURL()) != 0;
}

void StationURL::Copy(const StationURL& rStationURL)
{
    if (rStationURL.m_bStrValid)
    {
        SetURL(rStationURL.GetURL());
        m_bStrValid = true;
    }
    else
    {
        SetURL(0);
    }

    if (rStationURL.m_bDataValid)
    {
        *m_pAddress = *rStationURL.m_pAddress;
        m_URLType = rStationURL.m_URLType;
        m_uintParams = rStationURL.m_uintParams;
        m_strParams = rStationURL.m_strParams;
        m_fragmentParams = rStationURL.m_fragmentParams;
        m_bDataValid = true;
    }

    m_bValid = rStationURL.m_bValid;
}

void StationURL::SetURL(const wchar_t* pURL)
{
    if (m_pURL != 0)
    {
        SpecialDeleteArray(m_pURL);
        m_bStrValid = false;
        m_pURL = 0;
    }

    if (pURL != 0)
    {
        const unsigned int uiLength = static_cast<unsigned int>(std::wcslen(pURL)) + 1;

        m_pURL = SpecialNewArray<wchar_t>(uiLength, L"", 0);
        std::wcsncpy(m_pURL, pURL, uiLength);
        m_bDataValid = false;
        m_bValid = false;
        m_pURL[uiLength - 1] = L'\0';
        m_bStrValid = true;
    }
}

const wchar_t* StationURL::GetURL() const
{
    ValidateStr();
    return m_pURL;
}

void StationURL::SetURLType(URLType type)
{
    ValidateData();
    m_URLType = type;
    m_bStrValid = false;
}

StationURL::URLType StationURL::GetURLType() const
{
    ValidateData();
    return m_URLType;
}

bool StationURL::SetAddress(const wchar_t* pAddress)
{
    ValidateData();
    m_bStrValid = false;
    return m_pAddress->SetAddress(pAddress);
}

String StationURL::GetAddress() const
{
    wchar_t buffer[ADDRESS_BUFFER_LENGTH];

    buffer[0] = L'\0';
    ValidateData();

    if (m_pAddress->GetAddress() != 0)
    {
        m_pAddress->GetAddress(buffer, ADDRESS_BUFFER_LENGTH);
    }

    return String(buffer);
}

void StationURL::SetPortNumber(unsigned short uiPortNumber)
{
    ValidateData();
    m_pAddress->SetPortNumber(uiPortNumber);
    m_bStrValid = false;
}

unsigned short StationURL::GetPortNumber() const
{
    ValidateData();
    return m_pAddress->GetPortNumber();
}

bool StationURL::SetInetAddress(const InetAddress* pAddress)
{
    ValidateData();
    *m_pAddress = *pAddress;
    m_bStrValid = false;
    return true;
}

const InetAddress* StationURL::GetInetAddress() const
{
    ValidateData();
    return m_pAddress;
}

bool StationURL::Set(const String& rName, const String& rValue, bool bFragment)
{
    ValidateData();

    if (bFragment)
    {
        m_fragmentParams[rName] = rValue;
    }
    else
    {
        m_strParams[rName] = rValue;
    }

    m_bStrValid = false;
    return true;
}

qResult StationURL::SetParamValue(const wchar_t* pName, unsigned int uiValue)
{
    ValidateData();

    m_strParams.erase(String(pName));
    m_uintParams[String(pName)] = uiValue;
    m_bStrValid = false;

    return qResult(URL_RESULT_SUCCESS);
}

unsigned int StationURL::GetParamValue(const wchar_t* pName, unsigned int uiDefault) const
{
    ValidateData();

    UIntParamMap::const_iterator itValue = m_uintParams.find(String(pName));

    if (itValue == m_uintParams.end())
    {
        StrParamMap::const_iterator itString = m_strParams.find(String(pName));

        if (itString != m_strParams.end())
        {
            uiDefault = static_cast<unsigned int>(std::wcstoul(itString->second, 0, 10));
            const_cast<StationURL*>(this)->SetParamValue(pName, uiDefault);
        }
    }
    else
    {
        uiDefault = itValue->second;
    }

    return uiDefault;
}

void StationURL::SetStreamID(unsigned char uiStreamID)
{
    SetParamValue(PARAM_STREAM_ID, uiStreamID);
}

unsigned char StationURL::GetStreamID() const
{
    return static_cast<unsigned char>(GetParamValue(PARAM_STREAM_ID, 0));
}

void StationURL::SetStreamType(Stream::Type type)
{
    SetParamValue(PARAM_STREAM_TYPE, static_cast<unsigned int>(type));
}

Stream::Type StationURL::GetStreamType() const
{
    return static_cast<Stream::Type>(GetParamValue(PARAM_STREAM_TYPE, 0));
}

void StationURL::SetPrincipalID(unsigned int uiPrincipalID)
{
    SetParamValue(PARAM_PRINCIPAL_ID, uiPrincipalID);
}

unsigned int StationURL::GetPrincipalID() const
{
    return GetParamValue(PARAM_PRINCIPAL_ID, 0);
}

void StationURL::SetConnectionID(unsigned int uiConnectionID)
{
    SetParamValue(PARAM_CONNECTION_ID, uiConnectionID);
}

unsigned int StationURL::GetConnectionID() const
{
    return GetParamValue(PARAM_CONNECTION_ID, 0);
}

void StationURL::SetRVConnectionID(unsigned int uiConnectionID)
{
    SetParamValue(PARAM_RV_CONNECTION_ID, uiConnectionID);
}

unsigned int StationURL::GetRVConnectionID() const
{
    return GetParamValue(PARAM_RV_CONNECTION_ID, 0);
}

void StationURL::SetProbeRequestID(unsigned int uiProbeRequestID)
{
    SetParamValue(PARAM_PROBE_REQUEST_ID, uiProbeRequestID);
}

unsigned int StationURL::GetProbeRequestID() const
{
    return GetParamValue(PARAM_PROBE_REQUEST_ID, 0);
}

void StationURL::SetFastProbeResponse(bool bFastProbeResponse)
{
    SetParamValue(PARAM_FAST_PROBE_RESPONSE, bFastProbeResponse);
}

bool StationURL::GetFastProbeResponse() const
{
    return GetParamValue(PARAM_FAST_PROBE_RESPONSE, 0) != 0;
}

void StationURL::SetNATMapping(NATProperties::MappingProperties mapping)
{
    SetParamValue(PARAM_NAT_MAPPING, static_cast<unsigned int>(mapping));
}

NATProperties::MappingProperties StationURL::GetNATMapping() const
{
    return static_cast<NATProperties::MappingProperties>(GetParamValue(PARAM_NAT_MAPPING, 0));
}

void StationURL::SetNATFiltering(NATProperties::FilteringProperties filtering)
{
    SetParamValue(PARAM_NAT_FILTERING, static_cast<unsigned int>(filtering));
}

NATProperties::FilteringProperties StationURL::GetNATFiltering() const
{
    return static_cast<NATProperties::FilteringProperties>(GetParamValue(PARAM_NAT_FILTERING, 0));
}

void StationURL::SetProbeRequestInitiation(bool bProbeRequestInitiation)
{
    SetParamValue(PARAM_PROBE_REQUEST_INITIATION, bProbeRequestInitiation);
}

bool StationURL::GetProbeRequestInitiation() const
{
    return GetParamValue(PARAM_PROBE_REQUEST_INITIATION, 0) != 0;
}

void StationURL::SetUPnPSupport(bool bSupported)
{
    SetParamValue(PARAM_UPNP_SUPPORT, bSupported);
}

void StationURL::SetNatPMPSupport(bool bSupported)
{
    SetParamValue(PARAM_NAT_PMP_SUPPORT, bSupported);
}

void StationURL::SetType(unsigned int uiType)
{
    SetParamValue(PARAM_TYPE, uiType);
}

unsigned int StationURL::GetType() const
{
    return GetParamValue(PARAM_TYPE, 0);
}

void StationURL::SetRelayServerAddress(const String& rAddress)
{
    m_strParams.erase(String(PARAM_RELAY_SERVER_ADDRESS));
    m_strParams[String(PARAM_RELAY_SERVER_ADDRESS)] = rAddress;
}

String StationURL::GetRelayServerAddress() const
{
    StrParamMap::const_iterator itString = m_strParams.find(String(PARAM_RELAY_SERVER_ADDRESS));

    if (itString != m_strParams.end())
    {
        return itString->second;
    }

    return String(L"");
}

void StationURL::SetRelayServerPort(unsigned short uiPortNumber)
{
    SetParamValue(PARAM_RELAY_SERVER_PORT, uiPortNumber);
}

unsigned short StationURL::GetRelayServerPort() const
{
    return static_cast<unsigned short>(GetParamValue(PARAM_RELAY_SERVER_PORT, 0));
}

void StationURL::SetRelayAddress(const String& rAddress)
{
    m_strParams.erase(String(PARAM_RELAY_ADDRESS));
    m_strParams[String(PARAM_RELAY_ADDRESS)] = rAddress;
}

String StationURL::GetRelayAddress() const
{
    StrParamMap::const_iterator itString = m_strParams.find(String(PARAM_RELAY_ADDRESS));

    if (itString != m_strParams.end())
    {
        return itString->second;
    }

    return String(L"");
}

void StationURL::SetRelayPort(unsigned short uiPortNumber)
{
    SetParamValue(PARAM_RELAY_PORT, uiPortNumber);
}

unsigned short StationURL::GetRelayPort() const
{
    return static_cast<unsigned short>(GetParamValue(PARAM_RELAY_PORT, 0));
}

void StationURL::SetUseRelayServer(bool bUseRelayServer)
{
    SetParamValue(PARAM_USE_RELAY_SERVER, bUseRelayServer);
}

bool StationURL::GetUseRelayServer() const
{
    return GetParamValue(PARAM_USE_RELAY_SERVER, 0) != 0;
}

void StationURL::SetPlatformType()
{
    SetParamValue(PARAM_PLATFORM_TYPE, PLATFORM_TYPE);
}

bool StationURL::IsValid() const
{
    ValidateData();
    return m_bValid;
}

bool StationURL::IsMatching(const StationURL& rStationURL) const
{
    bool bMatching = false;

    const String address = GetAddress();
    const String otherAddress = rStationURL.GetAddress();

    if (String::IsEqual(address, otherAddress))
    {
        if (GetPortNumber() == rStationURL.GetPortNumber())
        {
            bMatching = true;
        }
    }

    return bMatching;
}

void StationURL::Trace(unsigned long long) const
{
}

void StationURL::ValidateData() const
{
    if (m_bDataValid)
    {
        return;
    }

    const_cast<StationURL*>(this)->Parse();
}

void StationURL::ValidateStr() const
{
    if (m_bStrValid)
    {
        return;
    }

    const_cast<StationURL*>(this)->Format();
}

qResult StationURL::CopyInSuppliedBuffer(wchar_t* pDestination, unsigned int uiSize, const wchar_t* pSource,
                                         unsigned int uiLength)
{
    if (pDestination == 0)
    {
        return qResult(URL_RESULT_INVALID_ARGUMENT);
    }

    if (uiSize < uiLength + 1)
    {
        return qResult(URL_RESULT_BUFFER_TOO_SMALL);
    }

    std::memcpy(pDestination, pSource, uiLength * sizeof(wchar_t));
    pDestination[uiLength] = L'\0';

    return qResult(URL_RESULT_SUCCESS);
}

qResult StationURL::GetURLType(wchar_t* pBuffer, unsigned int uiSize) const
{
    if (m_pURL == 0)
    {
        return qResult(URL_RESULT_INVALID_POINTER);
    }

    if (pBuffer == 0)
    {
        return qResult(URL_RESULT_INVALID_ARGUMENT);
    }

    const wchar_t* pSeparator = std::wcsstr(m_pURL, L":/");

    if (pSeparator == 0)
    {
        return qResult(URL_RESULT_INVALID_URL);
    }

    if (pSeparator == m_pURL)
    {
        return qResult(URL_RESULT_INVALID_URL);
    }

    const unsigned int uiLength = static_cast<unsigned int>(pSeparator - m_pURL);

    for (unsigned int uiIndex = 0; uiIndex < uiLength; ++uiIndex)
    {
        const unsigned int uiChar = static_cast<unsigned int>(m_pURL[uiIndex]);

        if (!((uiChar >= L'A' && uiChar <= L'Z') || (uiChar >= L'a' && uiChar <= L'z')
              || (uiChar >= L'0' && uiChar <= L'9') || uiChar == L'-' || uiChar == L'_'))
        {
            return qResult(URL_RESULT_INVALID_URL);
        }
    }

    return CopyInSuppliedBuffer(pBuffer, uiSize, m_pURL, uiLength);
}

StationURL::URLType StationURL::ParseURLType() const
{
    wchar_t buffer[URL_TYPE_BUFFER_LENGTH];

    const qResult result = GetURLType(buffer, URL_TYPE_BUFFER_LENGTH);

    if (result.Equals(true))
    {
        if (std::wcscmp(buffer, L"prudp") == 0)
        {
            return Prudp;
        }

        if (std::wcscmp(buffer, L"prudps") == 0)
        {
            return Prudps;
        }

        if (std::wcscmp(buffer, L"udp") == 0)
        {
            return Udp;
        }
    }

    return Unknown;
}

bool StationURL::ParseParam(wchar_t* pParam, bool bFragment)
{
    wchar_t* pSeparator = std::wcsstr(pParam, L"=");

    if (pSeparator == 0 || pParam == pSeparator)
    {
        return false;
    }

    wchar_t* pValue = pSeparator + 1;

    if (*pValue == L'\0')
    {
        return false;
    }

    if (std::wcsstr(pValue, L"=") != 0)
    {
        return false;
    }

    bool bParsed = true;

    *pSeparator = L'\0';

    if (!bFragment)
    {
        if (std::wcscmp(pParam, PARAM_ADDRESS) == 0)
        {
            bParsed = SetAddress(pValue);
        }
        else if (std::wcscmp(pParam, PARAM_PORT) == 0)
        {
            SetPortNumber(static_cast<unsigned short>(std::wcstoul(pValue, 0, 10)));
        }
        else
        {
            Set(String(pParam), String(pValue), false);
        }
    }
    else
    {
        Set(String(pParam), String(pValue), bFragment);
    }

    *pSeparator = L'=';
    return bParsed;
}

bool StationURL::ParseParams(wchar_t* pParams, bool bFragment)
{
    bool bLast = false;

    for (;;)
    {
        wchar_t* pSeparator = std::wcsstr(pParams, L";");

        if (pSeparator == 0)
        {
            bLast = true;
            pSeparator = pParams + std::wcslen(pParams);

            if (pParams == pSeparator)
            {
                return true;
            }
        }
        else if (pParams == pSeparator)
        {
            return false;
        }

        *pSeparator = L'\0';

        const bool bParsed = ParseParam(pParams, bFragment);

        if (!bLast)
        {
            *pSeparator = L';';
        }

        if (!bParsed)
        {
            return false;
        }

        pParams = pSeparator + 1;

        if (bLast)
        {
            return true;
        }
    }
}

void StationURL::Parse()
{
    m_bDataValid = true;

    m_pAddress->Init();
    m_uintParams.clear();
    m_strParams.clear();
    m_fragmentParams.clear();

    m_URLType = ParseURLType();

    if (m_URLType == Unknown)
    {
        return;
    }

    wchar_t* pStart = std::wcsstr(m_pURL, L":/") + std::wcslen(L":/");
    wchar_t* pFragment = std::wcsstr(pStart, L"#");
    bool bParsed;

    if (pFragment == 0)
    {
        bParsed = ParseParams(pStart, false);
    }
    else
    {
        if (std::wcsstr(pFragment + 1, L"#") != 0)
        {
            return;
        }

        *pFragment = L'\0';
        bParsed = ParseParams(pStart, false);
        *pFragment = L'#';

        if (!bParsed)
        {
            return;
        }

        bParsed = ParseParams(pFragment + 1, true);
    }

    if (bParsed)
    {
        m_bStrValid = true;
        m_bValid = true;
    }
}

void StationURL::Format()
{
    wchar_t buffer[URL_BUFFER_LENGTH];
    int iLength = 0;

    if (m_URLType == Prudp)
    {
        iLength = std::swprintf(buffer, URL_BUFFER_LENGTH, L"prudp:/");
    }
    else if (m_URLType == Prudps)
    {
        iLength = std::swprintf(buffer, URL_BUFFER_LENGTH, L"prudps:/");
    }
    else if (m_URLType == Udp)
    {
        iLength = std::swprintf(buffer, URL_BUFFER_LENGTH, L"udp:/");
    }

    bool bNeedSeparator = false;

    if (m_pAddress->GetAddress() != 0)
    {
        iLength += std::swprintf(&buffer[iLength], URL_BUFFER_LENGTH, L"%ls%ls", PARAM_ADDRESS, L"=");
        m_pAddress->GetAddress(&buffer[iLength], URL_BUFFER_LENGTH - iLength);
        iLength += static_cast<int>(std::wcslen(&buffer[iLength]));
        bNeedSeparator = true;
    }

    if (m_pAddress->GetPortNumber() != 0)
    {
        if (bNeedSeparator)
        {
            iLength += std::swprintf(&buffer[iLength], URL_BUFFER_LENGTH, L";");
        }

        iLength += std::swprintf(&buffer[iLength], URL_BUFFER_LENGTH, L"%ls%ls%u", PARAM_PORT, L"=",
                                 m_pAddress->GetPortNumber());
        bNeedSeparator = true;
    }

    StrParamMap::const_iterator itString = m_strParams.begin();
    UIntParamMap::const_iterator itValue = m_uintParams.begin();

    while (itString != m_strParams.end() || itValue != m_uintParams.end())
    {
        if (bNeedSeparator)
        {
            iLength += std::swprintf(&buffer[iLength], URL_BUFFER_LENGTH, L";");
        }

        const wchar_t* pName;
        const wchar_t* pStringValue = 0;
        unsigned int uiValue = 0;

        if (itString == m_strParams.end())
        {
            pName = itValue->first;
            uiValue = itValue->second;
            ++itValue;
        }
        else if (itValue == m_uintParams.end())
        {
            pName = itString->first;
            pStringValue = itString->second;
            ++itString;
        }
        else if (itString->first < itValue->first)
        {
            pName = itString->first;
            pStringValue = itString->second;
            ++itString;
        }
        else
        {
            pName = itValue->first;
            uiValue = itValue->second;
            ++itValue;
        }

        if (pStringValue == 0)
        {
            iLength += std::swprintf(&buffer[iLength], URL_BUFFER_LENGTH, L"%ls%ls%u", pName, L"=", uiValue);
        }
        else
        {
            iLength += std::swprintf(&buffer[iLength], URL_BUFFER_LENGTH, L"%ls%ls%ls", pName, L"=", pStringValue);
        }

        bNeedSeparator = true;
    }

    if (!m_fragmentParams.empty())
    {
        iLength += std::swprintf(&buffer[iLength], URL_BUFFER_LENGTH, L"#");
        bNeedSeparator = false;

        for (StrParamMap::const_iterator itFragment = m_fragmentParams.begin(); itFragment != m_fragmentParams.end();
             ++itFragment)
        {
            if (bNeedSeparator)
            {
                iLength += std::swprintf(&buffer[iLength], URL_BUFFER_LENGTH, L";");
            }

            iLength += std::swprintf(&buffer[iLength], URL_BUFFER_LENGTH, L"%ls%ls%ls",
                                     static_cast<const wchar_t*>(itFragment->first), L"=",
                                     static_cast<const wchar_t*>(itFragment->second));
            bNeedSeparator = true;
        }
    }

    SetURL(buffer);
    m_bDataValid = true;
    m_bValid = true;
}

}
}
