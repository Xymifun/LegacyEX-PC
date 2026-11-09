#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SOCKET_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SOCKET_HPP

#include "nn/nex/InetAddress.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/SocketDriver.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class IOCompletionContext;
class StringStream;

class Socket : public RootObject
{
public:
    enum State
    {
        STATE_OPEN = 0,
        STATE_BOUND = 1,
        STATE_CLOSED = 3
    };

    enum PollEvent
    {
        POLL_EVENT_READ = 1,
        POLL_EVENT_WRITE = 2
    };

    Socket(unsigned int uiTrafficType, unsigned int uiMTU);
    virtual ~Socket();

    virtual bool Bind(InetAddress* pAddress, bool bReuse);
    virtual int Recv(Buffer* pBuffer, InetAddress* pAddress);
    virtual int RecvNonBlock(Buffer* pBuffer, InetAddress* pAddress);
    virtual int Send(const Buffer& rBuffer, const InetAddress& rAddress);

    InetAddress* GetAddress();
    bool Open(bool bUDP);
    void Close();
    void Shutdown();
    void SetTTL(unsigned char uiTTL);
    bool Connect(InetAddress* pAddress, IOCompletionContext* pContext);
    bool SetMulticastAddress(unsigned int uiAddress);
    void Poll(PollEvent events, PollEvent* pResult, unsigned int uiTimeout);
    int RecvNonBlock(unsigned char* pBuffer, unsigned int uiSize, unsigned int* pReceived, InetAddress* pAddress);
    int Send(const unsigned char* pBuffer, unsigned int uiSize, const InetAddress& rAddress);
    void Trace(unsigned long long uiFilter);

protected:
    bool ProcessDriverSocketResult(SocketDriver::_Result result, IOCompletionContext* pContext);
    int RecvImpl(unsigned char* pBuffer, unsigned int uiSize, unsigned int* pReceived, InetAddress* pAddress,
                 SocketDriver::_SocketFlag flag);

    unsigned char m_uiState;
    InetAddress m_address;
    int m_iRecvAddrLen;
    bool m_bUDP;
    unsigned int m_uiTrafficType;
    unsigned int m_uiMTU;
    bool m_bVDP;
    SocketDriver::Socket* m_pDriverSocket;
    bool m_bConnected;

    friend class IOCompletionNotifier;
    friend StringStream& operator<<(StringStream& rStream, Socket& rSocket);
};

extern const unsigned int SOCKET_SUCCESS;

StringStream& operator<<(StringStream& rStream, Socket& rSocket);

}
}

#endif
