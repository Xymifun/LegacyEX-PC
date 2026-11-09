#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_QUEUINGSOCKET_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_QUEUINGSOCKET_HPP

#include "nn/nex/NetworkEmulator.hpp"
#include "nn/nex/PacketQueue.hpp"
#include "nn/nex/Socket.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class Packet;
class PRUDPMessageSelector;
class RootTransport;
class TransportBuffer;
class TransportStreamManager;

template <typename T, typename TPolicy>
class qChain;

template <typename T>
class ChainPolicyHistoryPacket;

class QueuingSocket : public Socket
{
public:
    typedef qChain<Packet*, ChainPolicyHistoryPacket<Packet*> > HistoryChain;

    QueuingSocket(unsigned int uiTrafficType, RootTransport* pTransport, unsigned int uiMTU,
                  TransportBuffer* pTransportBuffer);
    virtual ~QueuingSocket();

    virtual bool Bind(InetAddress* pAddress, bool bReuse);
    virtual int Recv(Buffer* pBuffer, InetAddress* pAddress);
    virtual int RecvNonBlock(Buffer* pBuffer, InetAddress* pAddress);
    virtual int Send(const Buffer& rBuffer, const InetAddress& rAddress);
    virtual bool IsRecvDataExists() const { return true; }

    virtual int SendBuffer(const Buffer& rBuffer, const InetAddress& rAddress);
    virtual int RecvBuffer(Buffer* pBuffer, InetAddress* pAddress);

    int OutputQueue(const Buffer& rBuffer, const InetAddress& rAddress);
    void RecvAndQueueIncomingBuffer(unsigned int uiBufferSize);
    void DequeueAndSendOutgoingBuffer();
    int FillPacketQueueFromBuffer(Buffer* pBuffer, const InetAddress* pAddress, PacketQueue* pQueue,
                                  TransportStreamManager* pStreamManager, bool* pFailed);
    Packet* ExtractIncomingPacket(TransportStreamManager* pStreamManager, const InetAddress* pAddress, Buffer* pBuffer,
                                  bool* pFailed);
    Buffer* CreateBufferFromPacketQueue(PacketQueue* pQueue, HistoryChain* pHistory, unsigned int uiMaxSize);

    static void RetransmitFromHistoryPacketQueue(PacketQueue* pQueue, HistoryChain* pHistory, unsigned short uiPort);

private:
    friend class QueuingSocketTransportBuffer;
    friend class SocketTransport;

    unsigned int m_unknown38;
    NetworkEmulator m_outputEmulator;
    NetworkEmulator m_inputEmulator;
    unsigned int m_unknownD0;
    PRUDPMessageSelector* m_pMessageSelector;
    PacketQueue m_sendingQueue;
    TransportBuffer* m_pTransportBuffer;
    bool m_unknownF8;
};

}
}

#endif
