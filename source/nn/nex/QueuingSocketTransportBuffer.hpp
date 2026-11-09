#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_QUEUINGSOCKETTRANSPORTBUFFER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_QUEUINGSOCKETTRANSPORTBUFFER_HPP

#include "nn/nex/QueuingSocket.hpp"
#include "nn/nex/TransportBufferSet.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class Condition;
class RootTransport;
class TransportBuffer;

class QueuingSocketTransportBuffer : public QueuingSocket
{
public:
    QueuingSocketTransportBuffer(unsigned int uiTrafficType, RootTransport* pTransport, unsigned int uiRecvCount,
                                 unsigned int uiSendCount, unsigned int uiMTU, Condition* pCondition,
                                 TransportBuffer* pTransportBuffer);
    virtual ~QueuingSocketTransportBuffer();

    virtual bool IsRecvDataExists() const;
    virtual int SendBuffer(const Buffer& rBuffer, const InetAddress& rAddress);
    virtual int RecvBuffer(Buffer* pBuffer, InetAddress* pAddress);

    void PushRecvBuffer(Buffer* pBuffer, const InetAddress& rAddress);
    bool RecvCoreSingle(bool bNonBlock);
    bool RecvCore(bool bNonBlock);
    bool SendBufferCoreSingle();
    bool SendBufferCore();

private:
    friend class SocketTransport;

    TransportBufferSet m_recvSet;
    Buffer* m_pRecvBuffer;
    TransportBufferSet m_sendSet;
    unsigned int m_uiRecvCount;
    unsigned int m_uiSendCount;
    Condition* m_pCondition;
};

}
}

#endif
