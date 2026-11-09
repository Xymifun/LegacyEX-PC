#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STATIONURL_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STATIONURL_HPP

#include <functional>
#include <map>
#include <utility>

#include "nn/nex/InetAddress.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/NATProperties.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/Stream.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class StringStream;

class StationURL : public RootObject
{
public:
    enum URLType
    {
        Unknown = 0,
        Prudp = 1,
        Prudps = 2,
        Udp = 3
    };

    typedef std::map<String, unsigned int, std::less<String>, MemAllocator<std::pair<const String, unsigned int> > >
        UIntParamMap;
    typedef std::map<String, String, std::less<String>, MemAllocator<std::pair<const String, String> > > StrParamMap;

    StationURL();
    StationURL(const StationURL& rStationURL);
    StationURL(const String& rURL);
    StationURL(const wchar_t* pURL);
    virtual ~StationURL();

    StationURL& operator=(const StationURL& rStationURL);
    StationURL& operator=(const String& rURL);
    StationURL& operator=(const wchar_t* pURL);
    bool operator==(const StationURL& rStationURL) const;
    bool operator!=(const StationURL& rStationURL) const;

    void SetURL(const wchar_t* pURL);
    const wchar_t* GetURL() const;

    void SetURLType(URLType type);
    URLType GetURLType() const;

    bool SetAddress(const wchar_t* pAddress);
    String GetAddress() const;
    void SetPortNumber(unsigned short uiPortNumber);
    unsigned short GetPortNumber() const;

    bool SetInetAddress(const InetAddress* pAddress);
    const InetAddress* GetInetAddress() const;

    bool Set(const String& rName, const String& rValue, bool bFragment);
    bool Get(const String& rName, String* pValue, bool bFragment) const;
    bool Remove(const String& rName, bool bFragment);
    unsigned int GetNbParam(bool bFragment);

    qResult SetParamValue(const wchar_t* pName, unsigned int uiValue);
    qResult SetParamValue(const wchar_t* pName, const unsigned char* pValue, unsigned int uiSize);
    unsigned int GetParamValue(const wchar_t* pName, unsigned int uiDefault) const;
    bool GetParamValue(const wchar_t* pName, unsigned char* pValue, unsigned int uiSize) const;

    void SetStreamID(unsigned char uiStreamID);
    unsigned char GetStreamID() const;

    void SetStreamType(Stream::Type type);
    Stream::Type GetStreamType() const;

    void SetPrincipalID(unsigned int uiPrincipalID);
    unsigned int GetPrincipalID() const;

    void SetConnectionID(unsigned int uiConnectionID);
    unsigned int GetConnectionID() const;

    void SetRVConnectionID(unsigned int uiConnectionID);
    unsigned int GetRVConnectionID() const;

    void SetProbeRequestID(unsigned int uiProbeRequestID);
    unsigned int GetProbeRequestID() const;

    void SetFastProbeResponse(bool bFastProbeResponse);
    bool GetFastProbeResponse() const;

    void SetNATMapping(NATProperties::MappingProperties mapping);
    NATProperties::MappingProperties GetNATMapping() const;

    void SetNATFiltering(NATProperties::FilteringProperties filtering);
    NATProperties::FilteringProperties GetNATFiltering() const;

    void SetProbeRequestInitiation(bool bProbeRequestInitiation);
    bool GetProbeRequestInitiation() const;

    void SetUPnPSupport(bool bSupported);
    bool GetUPnPSupport() const;

    void SetNatPMPSupport(bool bSupported);
    bool GetNatPMPSupport() const;

    void SetType(unsigned int uiType);
    unsigned int GetType() const;

    void SetNodeId(unsigned short uiNodeId);
    unsigned short GetNodeId() const;

    void SetRelayServerAddress(const String& rAddress);
    String GetRelayServerAddress() const;
    void SetRelayServerPort(unsigned short uiPortNumber);
    unsigned short GetRelayServerPort() const;

    void SetRelayAddress(const String& rAddress);
    String GetRelayAddress() const;
    void SetRelayPort(unsigned short uiPortNumber);
    unsigned short GetRelayPort() const;

    void SetUseRelayServer(bool bUseRelayServer);
    bool GetUseRelayServer() const;

    void SetPlatformType();
    unsigned int GetPlatformType() const;

    bool IsValid() const;
    bool IsMatching(const StationURL& rStationURL) const;
    bool IsAddressMatching(const StationURL& rStationURL) const;

    void Trace(unsigned long long uiFilter) const;

private:
    void Copy(const StationURL& rStationURL);
    void Parse();
    void Format();
    void ValidateData() const;
    void ValidateStr() const;

    URLType ParseURLType() const;
    qResult GetURLType(wchar_t* pBuffer, unsigned int uiSize) const;
    bool ParseParam(wchar_t* pParam, bool bFragment);
    bool ParseParams(wchar_t* pParams, bool bFragment);

    static qResult CopyInSuppliedBuffer(wchar_t* pDestination, unsigned int uiSize, const wchar_t* pSource,
                                        unsigned int uiLength);

    InetAddress* m_pAddress;
    URLType m_URLType;
    UIntParamMap m_uintParams;
    StrParamMap m_strParams;
    StrParamMap m_fragmentParams;
    wchar_t* m_pURL;
    bool m_bDataValid;
    bool m_bStrValid;
    bool m_bValid;
};

StringStream& operator<<(StringStream& rStream, const StationURL& rURL);

}
}

#endif
