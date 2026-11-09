#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SECUREENDPOINT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SECUREENDPOINT_HPP

#include <stdint.h>

#include "nn/nex/EndPoint.hpp"
#include "nn/nex/UserContext.hpp"

namespace nn
{
namespace nex
{

class SecureEndPoint : public EndPoint
{
public:
    SecureEndPoint(ConnectionOrientedStream* pStream, const StationURL* pStationURL);
    virtual ~SecureEndPoint();

    virtual void ResetEndPointInfo() {}

    virtual bool GetEndPointInfo(EndPointInfo*) const { return false; }

    virtual unsigned int GetReliableBufferPacketCount() const { return 0; }

    virtual void GetReliableBufferPacketCounts(qVector<unsigned int>&) const {}

    virtual unsigned char GetMaxSubStreamID() const { return 0; }

    virtual bool IsNotConnected();
    virtual bool IsConnecting();
    virtual bool IsDisconnecting();
    virtual bool IsFaulty();
    virtual bool IsConnected();
    virtual bool PeerIsConnected();
    virtual bool PeerIsDisconnected();
    virtual void SetKeepAliveTimeout(unsigned int uiTimeout);
    virtual void SetMaxSilenceTime(unsigned int uiSilenceTime);
    virtual unsigned int GetKeepAliveTimeout();
    virtual unsigned int GetMaxSilenceTime();
    virtual void SetPeerConnected();
    virtual void SetPeerDisconnected();
    virtual _ConnectionState GetConnectionState() const;
    virtual bool SetConnectionState(_ConnectionState state);
    virtual qResult RegisterEventHandler(EndPointEventHandler* pEventHandler);
    virtual bool IsAnyPacketProcessed();
    virtual unsigned int GetSignaledFaultError() const;
    virtual unsigned int GetRTT() const;
    virtual unsigned int GetRTTAverage() const;
    virtual qResult _Connect(Buffer* pRequestData, Buffer* pResponseData, ConnectionCallback pCallback,
                             const UserContext& rUserContext, unsigned int uiTimeout);
    virtual qResult _Disconnect(ConnectionCallback pCallback, const UserContext& rUserContext, unsigned int uiTimeout);
    virtual qResult _Send(Buffer* pBuffer, unsigned int uiSize, bool bReliable, unsigned char uiSubStreamID,
                          unsigned int uiFlags);
    virtual unsigned long long GetSupportedFunctionsFlags() const;
    virtual void SignalFaultEvent(unsigned int uiError, bool bDisconnected);
    virtual void SetEncryptionKey(const Key& rKey);
    virtual Key& GetEncryptionKey();

    EndPoint* GetAssociatedEndPoint();
    void SetAssociatedEndPoint(EndPoint* pEndPoint);
    void CompleteDisconnect();

    static void DisconnectCallback(EndPoint* pEndPoint, qResult result, const UserContext* pUserContext);

private:
    EndPoint* m_pAssociatedEndPoint;
    ConnectionCallback m_pDisconnectCallback;
    UserContext m_disconnectContext;
};

}
}

#endif
