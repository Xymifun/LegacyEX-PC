#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTADAPTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTADAPTER_HPP

#include "nn/nex/InetAddress.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class CallContext;
class InterfaceInfo;
class SocketDriver;
class StationURL;

template <typename T>
class qProtectedList;

class TransportAdapter : public RootObject
{
public:
    enum ETransportMode
    {
        TRANSPORT_MODE_0 = 0,
        TRANSPORT_MODE_1 = 1
    };

    TransportAdapter();
    virtual ~TransportAdapter();

    virtual bool GetLocalURLs(qList<StationURL>* pURLs);
    virtual void ResolveAddress(CallContext* pCallContext, StationURL* pStationURL);
    virtual void ReleaseAddress(const StationURL& rStationURL);
    virtual void SortAndFilterTargetURLs(const qProtectedList<StationURL>& rTargetURLs, qList<StationURL>* pURLs);
    virtual bool IsExportableURL(const StationURL& rStationURL);
    virtual unsigned int GetUniqueMachineID(const StationURL& rStationURL) const;
    virtual void Load();
    virtual void Unload();
    virtual bool SetNetworkInterfaceInfo(InterfaceInfo* pInterfaceInfo);
    virtual bool SetLocalAddress(const String& rAddress);
    virtual bool SetPortNumber(unsigned short uiPortNumber);

    void SetTransportMode(ETransportMode mode);

    ETransportMode GetTransportMode() const { return m_transportMode; }

    const InetAddress& GetBroadcastAddress() const { return m_broadcastAddress; }

    const InetAddress& GetMulticastAddress() const { return m_multicastAddress; }

    const InetAddress& GetLocalAddress() const { return m_localAddress; }

    SocketDriver* GetSocketDriver() const { return m_pSocketDriver; }
    void Trace(unsigned long long uiFilter);

    static bool IsSendMultiAddress(const InetAddress* pAddress);
    static void TraceInetAddress(unsigned long long uiFilter, InetAddress* pAddress);

protected:
    ETransportMode m_transportMode;
    InetAddress m_broadcastAddress;
    InetAddress m_multicastAddress;
    InetAddress m_localAddress;
    SocketDriver* m_pSocketDriver;
};

extern TransportAdapter g_defaultTransportAdapter;
extern TransportAdapter* g_transportAdapter;

}
}

#endif
