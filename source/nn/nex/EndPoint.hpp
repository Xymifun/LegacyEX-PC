#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ENDPOINT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ENDPOINT_HPP

#include <stdint.h>

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/qList.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class ClientProtocolRequestBroker;
class ConnectionManager;
class ConnectionOrientedStream;
class EndPointEventHandler;
class EndPointInfo;
class FirstErrorHolder;
class Key;
class UserContext;
class Protocol;
class ProtocolRequestBroker;
class SecureEndPoint;

class EndPointInfoInterface
{
public:
    virtual ~EndPointInfoInterface();
};

class EndPoint : public EndPointInfoInterface, public RootObject
{
public:
    enum _ConnectionState
    {
        NotConnected = 0,
        Connecting = 1,
        Connected = 2,
        Disconnecting = 3,
        Faulty = 4
    };

    enum _BroadcastState
    {
        BroadcastDisabled = 0,
        BroadcastEnabled = 1,
        BroadcastUnrestricted = 2
    };

    typedef void (*ConnectionCallback)(EndPoint* pEndPoint, qResult result, const UserContext* pUserContext);

    EndPoint(ConnectionOrientedStream* pStream, const StationURL* pStationURL);
    virtual ~EndPoint();

    virtual void ResetEndPointInfo() = 0;
    virtual bool GetEndPointInfo(EndPointInfo* pEndPointInfo) const = 0;
    virtual unsigned int GetReliableBufferPacketCount() const = 0;
    virtual void GetReliableBufferPacketCounts(qVector<unsigned int>& rCounts) const = 0;
    virtual unsigned char GetMaxSubStreamID() const = 0;
    virtual bool IsNotConnected() = 0;
    virtual bool IsConnecting() = 0;
    virtual bool IsDisconnecting() = 0;
    virtual bool IsFaulty() = 0;
    virtual bool IsConnected() = 0;
    virtual bool PeerIsConnected() = 0;
    virtual bool PeerIsDisconnected() = 0;
    virtual void DisableKeepAlive();
    virtual void SetKeepAliveTimeout(unsigned int uiTimeout) = 0;
    virtual void SetMaxSilenceTime(unsigned int uiSilenceTime) = 0;
    virtual unsigned int GetKeepAliveTimeout() = 0;
    virtual unsigned int GetMaxSilenceTime() = 0;
    virtual void SetPeerConnected() = 0;
    virtual void SetPeerDisconnected() = 0;
    virtual _ConnectionState GetConnectionState() const = 0;
    virtual bool SetConnectionState(_ConnectionState state) = 0;
    virtual qResult RegisterEventHandler(EndPointEventHandler* pEventHandler);
    virtual bool IsAnyPacketProcessed() = 0;
    virtual unsigned int GetSignaledFaultError() const = 0;
    virtual unsigned int GetRTT() const = 0;
    virtual unsigned int GetRTTAverage() const = 0;
    virtual qResult _Connect(Buffer* pRequestData, Buffer* pResponseData, ConnectionCallback pCallback,
                             const UserContext& rUserContext, unsigned int uiTimeout) = 0;
    virtual qResult _Disconnect(ConnectionCallback pCallback, const UserContext& rUserContext,
                                unsigned int uiTimeout) = 0;
    virtual qResult _Send(Buffer* pBuffer, unsigned int uiSize, bool bReliable, unsigned char uiSubStreamID,
                          unsigned int uiFlags) = 0;
    virtual unsigned long long GetSupportedFunctionsFlags() const = 0;
    virtual void SetErrorHolder(FirstErrorHolder* pErrorHolder);
    virtual void UnsetErrorHolder();
    virtual void SignalFaultEvent(unsigned int uiError, bool bDisconnected);
    virtual void SetEncryptionKey(const Key& rKey) = 0;
    virtual Key& GetEncryptionKey() = 0;

    void Open();
    void Close();

    void _Open();
    void _Close();

    void SetBroadcastState(_BroadcastState state);
    const wchar_t* GetConnectionStateString() const;

    void SetConnectionID(unsigned int uiConnectionID);
    void SetPrincipalID(unsigned int uiPrincipalID);

    unsigned int GetConnectionID() const { return m_uiConnectionID; }

    ConnectionOrientedStream* GetStream() const { return m_pStream; }

protected:
    friend class Protocol;
    friend class ClientProtocolRequestBroker;
    friend class ProtocolRequestBroker;
    friend class NATTraversalRelayClient;
    friend class SecureEndPoint;
    friend class ServerProtocol;
    friend class ConnectionManager;
    friend class GlobalNotificationEventManager;

    ConnectionOrientedStream* m_pStream;
    StationURL m_stationURL;
    EndPointEventHandler* m_pEventHandler;
    SecureEndPoint* m_pSecureEndPoint;
    unsigned int m_uiConnectionID;
    unsigned int m_uiPrincipalID;
    int m_iOpenCount;
    _BroadcastState m_BroadcastState;
    FirstErrorHolder* m_pErrorHolder;
    unsigned char m_uiCtorFlag;
};

class EndPointGroup : public RootObject
{
public:
    EndPointGroup();
    ~EndPointGroup();

private:
    qList<EndPoint*> m_lstEndPoints;
    qList<EndPointGroup*> m_lstEndPointGroups;
    CriticalSection m_csGroup;
};

}
}

#endif
