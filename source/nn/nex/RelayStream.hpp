#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RELAYSTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RELAYSTREAM_HPP

#include "nn/nex/InetAddress.hpp"
#include "nn/nex/RelayPingProtocolHeader.hpp"
#include "nn/nex/RelayType.hpp"
#include "nn/nex/RoutingTable.hpp"
#include "nn/nex/Stream.hpp"

namespace nn
{
namespace nex
{

const unsigned int RELAY_INVALID_ID = 0xFFFFFFFFu;

class Buffer;
class Packet;
class PacketOut;
class RootTransport;

class RelayStream : public Stream
{
public:
    typedef void (*PingCallback)(unsigned int* pContext, const InetAddress* pAddress, unsigned short uiPort,
                                 unsigned int uiTimeStamp, const RelayPingProtocolHeader* pHeader, Buffer* pPayload);

    RelayStream();
    virtual ~RelayStream() {}

    virtual bool ReceiveIncomingPacket(unsigned short uiPort, unsigned char uiStreamID, Packet* pPacket);
    virtual void DoWork();

    static PacketOut* BuildRelayPacket(Buffer* pPayload, RelayType type);

private:
    friend class Relay;

    RootTransport* m_pTransport;
    RoutingTable m_routingTable;
    unsigned int m_uiGameServerID;
    unsigned int m_uiRelayGatheringID;
    PingCallback m_pPingCallback;
    unsigned int m_unknown3C;
};

}
}

#endif
