#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SOCKETDRIVER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SOCKETDRIVER_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class SocketDriver : public RootObject
{
public:
    enum _Result
    {
        RESULT_SUCCESS = 0,
        RESULT_FAILURE = 1,
        RESULT_WOULD_BLOCK = 2,
        RESULT_IN_PROGRESS = 3,
        RESULT_CLOSED = 4
    };

    enum _TrafficType
    {
        TRAFFIC_DATAGRAM = 0,
        TRAFFIC_STREAM = 1
    };

    enum _SocketFlag
    {
        SOCKET_FLAG_NONE = 0,
        SOCKET_FLAG_NON_BLOCK = 1
    };

    struct InetAddress
    {
        InetAddress() : m_uiAddress(0), m_uiPort(0) {}

        InetAddress(unsigned int uiAddress, unsigned short uiPort) : m_uiAddress(uiAddress), m_uiPort(uiPort) {}

        unsigned int m_uiAddress;
        unsigned short m_uiPort;
    };

    class Socket : public RootObject
    {
    public:
        Socket() : m_bAsync(false) {}

        virtual bool Open(_TrafficType type) = 0;
        virtual void Close() = 0;
        virtual bool Bind(unsigned short& rPort) = 0;
        virtual _Result RecvFrom(unsigned char* pBuffer, unsigned int uiSize, InetAddress* pAddress,
                                 unsigned int* pReceived, _SocketFlag flag) = 0;
        virtual _Result SendTo(unsigned char* pBuffer, unsigned int uiSize, const InetAddress& rAddress,
                               unsigned int* pSent) = 0;
        virtual bool Shutdown() = 0;
        virtual _Result Connect(const InetAddress& rAddress) = 0;
        virtual _Result Recv(unsigned char* pBuffer, unsigned int uiSize, unsigned int* pReceived) = 0;
        virtual _Result Send(unsigned char* pBuffer, unsigned int uiSize, unsigned int* pSent) = 0;
        virtual bool SetMulticastAddress(unsigned int uiAddress) { return true; }

        virtual bool SetTTL(unsigned char uiTTL) { return false; }

        virtual ~Socket() {}

        virtual bool SetAsync(bool bAsync) = 0;

    protected:
        bool m_bAsync;
    };

    struct PollInfo
    {
        PollInfo() : m_pSocket(0), m_uiResult(0), m_uiEvents(0) {}

        Socket* m_pSocket;
        unsigned int m_uiEvents;
        unsigned int m_uiResult;
    };

    virtual ~SocketDriver() {}

    virtual Socket* Create() = 0;
    virtual void Delete(Socket* pSocket) = 0;
    virtual bool Poll(PollInfo* pInfo, unsigned int uiCount, unsigned int uiTimeout) = 0;
};

extern SocketDriver* g_defaultSocketDriver;
extern SocketDriver* g_socketDriver;

}
}

#endif
