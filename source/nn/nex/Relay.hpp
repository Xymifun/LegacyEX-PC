#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RELAY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RELAY_HPP

#include "nn/nex/InetAddress.hpp"
#include "nn/nex/RelayPingProtocolHeader.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class Network;
class RelayStream;
class RootTransport;
class StationURL;

class Relay : public RootObject
{
public:
    Relay();
    virtual ~Relay();

    void SetTransport(RootTransport* pTransport);
    void EnableRelay(bool bEnable);
    void UnregisterStream();
    void Clean();
    bool Exists();

    bool HasRelay(const InetAddress* pAddress, const InetAddress* pRelay);
    bool ShouldRelay(const InetAddress* pAddress);
    bool AddRelay(const InetAddress& rAddress, const InetAddress& rKey, bool bReplace);
    bool RelayMessage(Buffer* pPayload, const InetAddress* pDestination, unsigned short uiPort);
    bool PingMessage(Buffer* pPayload, const RelayPingProtocolHeader* pHeader, const InetAddress* pDestination,
                     unsigned short uiPort);

    void SetRelayGatheringID(unsigned int uiID);
    unsigned int GetRelayGatheringID();
    void SetGameServerID(unsigned int uiID);
    unsigned int GetGameServerID();
    void SetSrcPublicAddress(const StationURL& rURL);

    bool IsEnabled() const { return m_bEnabled; }

    const InetAddress& GetSrcPublicAddress() const { return m_address; }

private:
    RelayStream* m_pStream;
    bool m_bEnabled;
    bool m_bRegistered;
    Network* m_pNetwork;
    InetAddress m_address;
};

}
}

#endif
