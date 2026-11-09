#include "nn/nex/BerkeleySocketDriver.hpp"

#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "game/core/SendMultiParameter.hpp"
#include "nn/nex/Platform.hpp"
#include "nn/nex/PseudoGlobalVariable.hpp"
#include "nn/nex/PseudoSingleton.hpp"
#include "nn/nex/Time.hpp"
#include "vendor/__N_19_OnlineCore_Blob_cpp_9428d484.hpp"

extern "C"
{
int mw_socket(int family, int type, int protocol);
int socketlasterr();
}

namespace nn
{
namespace nex
{

namespace
{

enum
{
    SOCKET_BUFFER_SIZE = 32000,
    SOCKET_ADDRESS_SIZE = 16,
    FIRST_BIND_PORT = 0x2390,
    LAST_BIND_PORT = 0x23f0,
    SOCKET_ERROR_WOULD_BLOCK = 6,
    SOCKET_ERROR_IN_PROGRESS = 0x16,
    SOCKET_ERROR_CLOSED = 0x18,
    LEVEL_IP = 0,
    POLL_EVENT_READ = 1,
    POLL_EVENT_WRITE = 2
};

struct AnalyzeInfo
{
    unsigned int m_unknown0;
    unsigned int m_unknown4;
    unsigned int m_uiReceived;
    unsigned int m_uiReceiveCalls;
};

typedef void (*RecvFromCallback)(SocketDriver::_Result result, const unsigned char* pBuffer, unsigned int uiSize,
                                 const SocketDriver::InetAddress* pAddress, const unsigned int* pReceived);
typedef void (*SendToCallback)(SocketDriver::_Result result, const unsigned char* pBuffer, unsigned int uiSize,
                               const SocketDriver::InetAddress& rAddress, const unsigned int* pSent);
typedef void (*RecvCallback)(SocketDriver::_Result result, const unsigned char* pBuffer, unsigned int uiSize,
                             const unsigned int* pReceived);
typedef void (*SendCallback)(SocketDriver::_Result result, const unsigned char* pBuffer, unsigned int uiSize,
                             const unsigned int* pSent);

AnalyzeInfo s_analyzeInfo;
RecvFromCallback s_pfnRecvFromCallback = __N_19_OnlineCore_Blob_cpp_9428d484::DefaultRecvFromCallback;
SendToCallback s_pfnSendToCallback = __N_19_OnlineCore_Blob_cpp_9428d484::DefaultSendToCallback;
RecvCallback s_pfnRecvCallback = __N_19_OnlineCore_Blob_cpp_9428d484::DefaultRecvCallback;
SendCallback s_pfnSendCallback = __N_19_OnlineCore_Blob_cpp_9428d484::DefaultSendCallback;

PseudoGlobalVariable<SendMultiParameter> s_sendMultiParameter;

}

BerkeleySocketDriver::BerkeleySocket::BerkeleySocket()
    : m_iHandle(-1), m_pSendMultiParameter(&s_sendMultiParameter.Get())
{
}

BerkeleySocketDriver::BerkeleySocket::~BerkeleySocket()
{
}

unsigned int BerkeleySocketDriver::BerkeleySocket::GetLastSocketError(int iResult)
{
    return socketlasterr();
}

bool BerkeleySocketDriver::BerkeleySocket::Open(_TrafficType type)
{
    int iHandle;
    switch (type)
    {
    case TRAFFIC_DATAGRAM:
    {
        m_iHandle = mw_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        int iBufferSize = SOCKET_BUFFER_SIZE;
        setsockopt(m_iHandle, SOL_SOCKET, SO_SNDBUF, &iBufferSize, sizeof(iBufferSize));
        iBufferSize = SOCKET_BUFFER_SIZE;
        setsockopt(m_iHandle, SOL_SOCKET, SO_RCVBUF, &iBufferSize, sizeof(iBufferSize));
        iHandle = m_iHandle;
        break;
    }
    case TRAFFIC_STREAM:
        iHandle = mw_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        m_iHandle = iHandle;
        break;
    default:
        iHandle = m_iHandle;
        break;
    }

    if (iHandle == -1)
        return false;

    SetAsync(false);
    return true;
}

void BerkeleySocketDriver::BerkeleySocket::Close()
{
    closesocket(m_iHandle);
    m_iHandle = -1;
}

bool BerkeleySocketDriver::BerkeleySocket::Shutdown()
{
    int iResult = shutdown(m_iHandle, SHUT_RDWR);
    bool bFailed = false;
    if (iResult < 0)
        bFailed = true;

    return bFailed;
}

bool BerkeleySocketDriver::BerkeleySocket::Bind(unsigned short& rPort)
{
    if (rPort == 0)
    {
        rPort = FIRST_BIND_PORT;
        for (;;)
        {
            if (Bind(rPort))
                break;

            ++rPort;
            if (rPort > LAST_BIND_PORT)
            {
                rPort = 0;
                return false;
            }
        }
    }
    else
    {
        sockaddr_in address;
        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(rPort);
        address.sin_addr.s_addr = 0;
        if (bind(m_iHandle, reinterpret_cast<const sockaddr*>(&address), SOCKET_ADDRESS_SIZE) != 0)
            return false;

        if (rPort == 0)
        {
            socklen_t iAddressSize = SOCKET_ADDRESS_SIZE;
            if (getsockname(m_iHandle, reinterpret_cast<sockaddr*>(&address), &iAddressSize) < 0)
                return false;

            rPort = ntohs(address.sin_port);
        }
    }

    return true;
}

SocketDriver::_Result BerkeleySocketDriver::BerkeleySocket::LastSocketErrorToResult(const wchar_t* pContext,
                                                                                    int iResult)
{
    unsigned int uiError = GetLastSocketError(iResult);
    switch (uiError)
    {
    case SOCKET_ERROR_WOULD_BLOCK:
        return RESULT_WOULD_BLOCK;
    case SOCKET_ERROR_CLOSED:
        return RESULT_CLOSED;
    default:
        return RESULT_FAILURE;
    }
}

SocketDriver::_Result BerkeleySocketDriver::BerkeleySocket::RecvFrom(unsigned char* pBuffer, unsigned int uiSize,
                                                                     SocketDriver::InetAddress* pAddress,
                                                                     unsigned int* pReceived, _SocketFlag flag)
{
    sockaddr_in from;
    memset(&from, 0, sizeof(from));
    from.sin_family = AF_INET;
    socklen_t iAddressSize = SOCKET_ADDRESS_SIZE;
    int iFlags = 0;
    if (flag & SOCKET_FLAG_NON_BLOCK)
        iFlags = MSG_DONTWAIT;

    ++s_analyzeInfo.m_uiReceiveCalls;
    _Result result = RESULT_SUCCESS;
    int iReceived = recvfrom(m_iHandle, pBuffer, uiSize, iFlags, reinterpret_cast<sockaddr*>(&from), &iAddressSize);
    if (iReceived >= 0 || (result = LastSocketErrorToResult(L"Socket::RecvFrom", iReceived)) == RESULT_SUCCESS)
    {
        ++s_analyzeInfo.m_uiReceived;
        if (pAddress != 0)
        {
            pAddress->m_uiAddress = ntohl(from.sin_addr.s_addr);
            pAddress->m_uiPort = ntohs(from.sin_port);
        }

        if (pReceived != 0)
            *pReceived = iReceived;
    }

    s_pfnRecvFromCallback(result, pBuffer, uiSize, pAddress, pReceived);
    return result;
}

SocketDriver::_Result BerkeleySocketDriver::BerkeleySocket::SendTo(unsigned char* pBuffer, unsigned int uiSize,
                                                                   const SocketDriver::InetAddress& rAddress,
                                                                   unsigned int* pSent)
{
    sockaddr_in to;
    memset(&to, 0, sizeof(to));
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = htonl(rAddress.m_uiAddress);
    to.sin_port = htons(rAddress.m_uiPort);
    int iSent = sendto(m_iHandle, pBuffer, uiSize, 0, reinterpret_cast<const sockaddr*>(&to), SOCKET_ADDRESS_SIZE);
    _Result result;
    if (iSent < 0)
    {
        result = LastSocketErrorToResult(L"Socket::SendTo", iSent);
        s_pfnSendToCallback(result, pBuffer, uiSize, rAddress, pSent);
    }
    else
    {
        if (pSent != 0)
            *pSent = iSent;

        s_pfnSendToCallback(RESULT_SUCCESS, pBuffer, uiSize, rAddress, pSent);
        result = RESULT_SUCCESS;
    }

    return result;
}

SocketDriver::_Result BerkeleySocketDriver::BerkeleySocket::Connect(const SocketDriver::InetAddress& rAddress)
{
    sockaddr_in to;
    memset(&to, 0, sizeof(to));
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = htonl(rAddress.m_uiAddress);
    to.sin_port = htons(rAddress.m_uiPort);
    int iResult = connect(m_iHandle, reinterpret_cast<const sockaddr*>(&to), SOCKET_ADDRESS_SIZE);
    if (iResult < 0)
    {
        if (GetLastSocketError(iResult) == SOCKET_ERROR_IN_PROGRESS)
            return RESULT_IN_PROGRESS;

        return RESULT_FAILURE;
    }

    return RESULT_SUCCESS;
}

SocketDriver::_Result BerkeleySocketDriver::BerkeleySocket::Recv(unsigned char* pBuffer, unsigned int uiSize,
                                                                 unsigned int* pReceived)
{
    int iReceived = recv(m_iHandle, pBuffer, uiSize, MSG_DONTWAIT);
    _Result result;
    if (iReceived < 0)
    {
        result = LastSocketErrorToResult(L"Socket::Recv", iReceived);
        s_pfnRecvCallback(result, pBuffer, uiSize, pReceived);
    }
    else
    {
        if (pReceived != 0)
            *pReceived = iReceived;

        result = RESULT_SUCCESS;
        s_pfnRecvCallback(RESULT_SUCCESS, pBuffer, uiSize, pReceived);
    }

    return result;
}

SocketDriver::_Result BerkeleySocketDriver::BerkeleySocket::Send(unsigned char* pBuffer, unsigned int uiSize,
                                                                 unsigned int* pSent)
{
    int iSent = send(m_iHandle, pBuffer, uiSize, MSG_DONTWAIT);
    _Result result;
    if (iSent < 0)
    {
        result = LastSocketErrorToResult(L"Socket::Send", iSent);
        s_pfnSendCallback(result, pBuffer, uiSize, pSent);
    }
    else
    {
        if (pSent != 0)
            *pSent = iSent;

        result = RESULT_SUCCESS;
        s_pfnSendCallback(RESULT_SUCCESS, pBuffer, uiSize, pSent);
    }

    return result;
}

bool BerkeleySocketDriver::BerkeleySocket::SetMulticastAddress(unsigned int uiAddress)
{
    return true;
}

bool BerkeleySocketDriver::BerkeleySocket::SetTTL(unsigned char uiTTL)
{
    int iTTL = uiTTL;
    return setsockopt(m_iHandle, LEVEL_IP, IP_TTL, &iTTL, sizeof(iTTL)) != 0;
}

bool BerkeleySocketDriver::BerkeleySocket::SetAsync(bool bAsync)
{
    int iAsync = bAsync;
    if (setsockopt(m_iHandle, SOL_SOCKET, SO_NONBLOCK, &iAsync, sizeof(iAsync)) == -1)
        return false;

    m_bAsync = bAsync;
    return true;
}

BerkeleySocketDriver::~BerkeleySocketDriver()
{
}

SocketDriver::Socket* BerkeleySocketDriver::Create()
{
    return new BerkeleySocket();
}

void BerkeleySocketDriver::Delete(Socket* pSocket)
{
    delete pSocket;
}

bool BerkeleySocketDriver::Poll(PollInfo* pInfo, unsigned int uiCount, unsigned int uiTimeout)
{
    int iMaxHandle = 0;
    fd_set readSet;
    fd_set writeSet;
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    unsigned int i = 0;
    while (i < uiCount)
    {
        pInfo[i].m_uiResult = 0;
        if (pInfo[i].m_uiEvents & POLL_EVENT_READ)
        {
            int iHandle = static_cast<BerkeleySocket*>(pInfo[i].m_pSocket)->m_iHandle;
            FD_SET(iHandle, &readSet);
            if (iMaxHandle <= iHandle)
                iMaxHandle = iHandle;
        }

        if (pInfo[i].m_uiEvents & POLL_EVENT_WRITE)
        {
            int iHandle = static_cast<BerkeleySocket*>(pInfo[i].m_pSocket)->m_iHandle;
            FD_SET(iHandle, &writeSet);
            if (iMaxHandle <= iHandle)
                iMaxHandle = iHandle;
        }

        ++i;
    }

    Time now(Time::GetTime());
    Time deadline = now + static_cast<int>(uiTimeout);
    struct timeval zeroTimeout = {0, 0};
    fd_set savedReadSet = readSet;
    fd_set savedWriteSet = writeSet;
    do
    {
        Platform::YieldThread();
        if (select(iMaxHandle + 1, &readSet, &writeSet, 0, &zeroTimeout) > 0)
        {
            i = 0;
            while (i < uiCount)
            {
                int iHandle = static_cast<BerkeleySocket*>(pInfo[i].m_pSocket)->m_iHandle;
                if (FD_ISSET(iHandle, &readSet))
                    pInfo[i].m_uiResult = 1;

                if (FD_ISSET(iHandle, &writeSet))
                    pInfo[i].m_uiResult = 2;

                ++i;
            }

            return true;
        }

        readSet = savedReadSet;
        writeSet = savedWriteSet;
        Platform::Sleep(1);
        now = Time(Time::GetTime());
    } while (static_cast<unsigned long long>(now) < static_cast<unsigned long long>(deadline));

    return false;
}

void BerkeleySocketDriver::GetPeerAddress(nn::nex::InetAddress* pAddresses, unsigned int uiCapacity,
                                          unsigned int* pCount)
{
    SendMultiParameter& rParameter = s_sendMultiParameter.Get();
    if (uiCapacity < rParameter.m_uiPeerAddressCount)
    {
        *pCount = 0;
        return;
    }

    ScopedCS lock(&rParameter.m_lock);
    unsigned int i = 0;
    while (i < rParameter.m_uiPeerAddressCount)
    {
        const sockaddr_in& rAddress = rParameter.m_peerAddresses[i % rParameter.m_uiPeerAddressCount];
        pAddresses[i].SetAddress(ntohl(rAddress.sin_addr.s_addr));
        pAddresses[i].SetPortNumber(ntohs(rAddress.sin_port));
        ++i;
    }

    *pCount = rParameter.m_uiPeerAddressCount;
}

void BerkeleySocketDriver::SetPeerAddress(const nn::nex::InetAddress* pAddresses, unsigned int uiCount)
{
    SendMultiParameter& rParameter = s_sendMultiParameter.Get();
    if (uiCount <= PEER_ADDRESS_CAPACITY)
    {
        ScopedCS lock(&rParameter.m_lock);
        unsigned int i = 0;
        while (i < uiCount)
        {
            sockaddr_in address;
            memset(&address, 0, sizeof(address));
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = htonl(pAddresses[i].GetAddress());
            address.sin_port = htons(pAddresses[i].GetPortNumber());
            rParameter.m_peerAddresses[i] = address;
            ++i;
        }

        rParameter.m_uiPeerAddressCount = uiCount;
    }
}

void BerkeleySocketDriver::ResetAnalyzeInfo()
{
    memset(&s_analyzeInfo, 0, sizeof(s_analyzeInfo));
}

}
}
