#include "nn/pia/common/Socket.hpp"

#include "nn/pia/common/SocketAddress.hpp"

extern "C"
{
int mw_socket(int family, int type, int protocol);
int bind(int handle, const void* address, int addressSize);
int setsockopt(int handle, int level, int option, const void* value, int valueSize);
int recvfrom_ex(int handle, void* buffer, int size, int flags, const void* address, int* addressSize, void* ancillary,
                int ancillarySize);
int sendto(int handle, const void* buffer, int size, int flags, const void* address, int addressSize);
int sendto_multi(int handle, const void* buffer, int size, int flags, const void* addresses, int count);
int inet_aton(const char* text, unsigned int* address);
int socketclose(int handle);
int socketlasterr();
}

namespace nn
{
namespace pia
{
namespace common
{

namespace
{
enum
{
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cResultDisconnected = 0xc406,
    cResultBufferFull = 0x4c0d,
    cResultNoData = 0x2c09,
    cResultSocketUnknown = 0xc35,
    cFamilyInet = 2,
    cTypeDatagram = 2,
    cProtocolUdp = 0x11,
    cLevelSocket = -1,
    cLevelIp = 0,
    cOptionReuseAddress = 4,
    cOptionIpTtl = 4,
    cOptionReceiveBuffer = 0x1002,
    cReceiveBufferSize = 0x4000,
    cSockAddrInSize = 0x10,
    cReceiveFlags = 0x60,
    cReceiveAncillarySize = 1,
    cMulticastCountMax = 0x20,
    cErrorBadDescriptor = 10,
    cErrorClosed = 24,
    cErrorNoBuffer = 27,
    cErrorConnectionReset = 49,
    cErrorSendClosed = 19,
    cErrorSendAborted = 29
};

inline bool IsClosedSocketError(int error)
{
    return error == cErrorConnectionReset || error == cErrorClosed;
}

}

int GetSocketLastError()
{
    return socketlasterr();
}

Result convertCommonSocketErrorToResult(int error)
{
    switch (error)
    {
    case 2:
    case 8:
    case 9:
        return cResultDisconnected;
    case 4:
    case 0xb:
    case 0x10:
    case 0x13:
    case 0x15:
    case 0x18:
    case 0x1f:
    case 0x20:
    case 0x31:
        return cResultInvalidArgument;
    case 1:
    case 0x12:
    case 0x2b:
    case 0x33:
        return cResultInvalidState;
    case 0x14:
        return cResultSocketUnknown;
    case 0x2c:
        return cResultBufferFull;
    default:
        return cResultInvalidState;
    }
}

Result convertRecvSocketErrorToResult(int error)
{
    switch (error)
    {
    case 2:
    case 8:
    case 9:
        return cResultDisconnected;
    case 4:
    case 0xb:
    case 0xc:
    case 0x15:
    case 0x18:
    case 0x31:
        return ResultInvalidArgument();
    case 6:
    case 0x12:
    case 0x2c:
    case 0x32:
        return cResultNoData;
    case 0x2b:
        return cResultInvalidState;
    default:
        return cResultInvalidState;
    }
}

Result convertSendSocketErrorToResult(int error)
{
    switch (error)
    {
    case 2:
    case 7:
    case 8:
    case 9:
    case 0x1e:
        return cResultDisconnected;
    case 4:
    case 0xb:
    case 0xc:
    case 0xe:
    case 0x15:
    case 0x18:
    case 0x20:
    case 0x2d:
    case 0x2e:
    case 0x2f:
    case 0x30:
    case 0x31:
        return ResultInvalidArgument();
    case 0x27:
    case 0x28:
    case 0x29:
    case 0x2a:
    case 0x2b:
        return cResultInvalidState;
    case 1:
    case 6:
    case 0x12:
    case 0x2c:
    case 0x32:
        return cResultBufferFull;
    default:
        return cResultInvalidState;
    }
}

Socket::Socket() : m_address()
{
    m_handle = -1;
    m_openCount = 0;
    m_address.m_address = 0;
    m_address.m_port = 0;
}

Socket::~Socket()
{
}

Result Socket::Open(int type, int protocol)
{
    const int handle = mw_socket(cFamilyInet, type, protocol);
    if (handle < 0)
        return convertCommonSocketErrorToResult(GetSocketLastError());

    int receiveBufferSize = cReceiveBufferSize;
    if (setsockopt(handle, cLevelSocket, cOptionReceiveBuffer, &receiveBufferSize, sizeof(receiveBufferSize)) < 0)
    {
        const int error = socketlasterr();
        socketclose(handle);
        return convertCommonSocketErrorToResult(error);
    }
    m_handle = handle;
    ++m_openCount;
    return 0;
}

Result Socket::Open()
{
    return Open(cTypeDatagram, cProtocolUdp);
}

Result Socket::Close()
{
    if (m_handle < 0)
        return ResultInvalidArgument();

    const int closeResult = socketclose(m_handle);
    m_handle = -1;
    --m_openCount;
    if (closeResult < 0)
    {
        const int error = GetSocketLastError();
        if (error == cErrorBadDescriptor)
            return 0;
        if (error == cErrorClosed)
            return cResultInvalidState;
        return convertCommonSocketErrorToResult(error);
    }
    return 0;
}

Result Socket::SetReuseAddress(bool enabled)
{
    if (m_handle < 0)
        return ResultInvalidArgument();

    int value = enabled;
    if (setsockopt(m_handle, cLevelSocket, cOptionReuseAddress, &value, sizeof(value)) < 0)
        return convertCommonSocketErrorToResult(GetSocketLastError());
    return 0;
}

Result Socket::Bind(const InetAddress& address, bool reuseAddress)
{
    const int handle = m_handle;
    if (handle < 0)
        return cResultInvalidArgument;

    if (reuseAddress)
    {
        const Result result = SetReuseAddress(reuseAddress);
        if (result != 0)
            return result;
    }

    m_address = address;
    SocketAddress socketAddress;
    socketAddress.SetInetAddress(address);
    if (bind(handle, socketAddress.GetSockAddrInPointer(), cSockAddrInSize) < 0)
    {
        const int error = GetSocketLastError();
        if (IsClosedSocketError(error))
        {
            if (m_handle < 0)
                return cResultDisconnected;
            if (handle == m_handle)
                return cResultDisconnected;
        }
        return convertCommonSocketErrorToResult(error);
    }
    return 0;
}

Result Socket::Bind(unsigned short port, bool reuseAddress)
{
    InetAddress address;
    address.m_address = 0;
    address.m_port = port;
    return Bind(address, reuseAddress);
}

Result Socket::RecvFrom(unsigned char* buffer, int size, InetAddress* address, unsigned char* ancillary,
                        int* receivedSize)
{
    const int handle = m_handle;
    if (handle < 0)
        return ResultInvalidArgument();

    SocketAddress socketAddress;
    int addressSize = cSockAddrInSize;
    const int received = recvfrom_ex(handle, buffer, size, cReceiveFlags, socketAddress.GetSockAddrInPointer(),
                                     &addressSize, ancillary, cReceiveAncillarySize);
    if (received < 1)
    {
        if (received < 0)
        {
            const int error = GetSocketLastError();
            if (IsClosedSocketError(error))
            {
                if (m_handle < 0)
                    return cResultDisconnected;
                if (handle != m_handle)
                    return cResultNoData;
                return cResultDisconnected;
            }
            if (error == cErrorNoBuffer)
                return cResultDisconnected;
            const Result result = convertRecvSocketErrorToResult(error);
            return result;
        }
        return cResultNoData;
    }

    if (address != 0)
    {
        socketAddress.GetInetAddress(address);
        address->Trace(8);
    }
    if (receivedSize != 0)
        *receivedSize = received;
    return 0;
}

Result Socket::SendTo(const void* buffer, int size, const InetAddress& address, int* sentSize)
{
    SocketAddress socketAddress;
    socketAddress.SetInetAddress(address);
    const int handle = m_handle;
    if (handle < 0)
        return cResultInvalidArgument;

    const int sent = sendto(handle, buffer, size, 0, socketAddress.GetSockAddrInPointer(), cSockAddrInSize);
    if (sent < 0)
    {
        const unsigned int error = GetSocketLastError();
        switch (error)
        {
        case cErrorClosed:
        case cErrorConnectionReset:
            if (m_handle < 0)
                return cResultDisconnected;
            if (handle != m_handle)
                return cResultBufferFull;
            return cResultDisconnected;
        case cErrorSendClosed:
            return cResultDisconnected;
        case cErrorSendAborted:
        case 0x2d:
        case 0x2e:
        case 0x2f:
            return cResultDisconnected;
        case cErrorNoBuffer:
            return cResultBufferFull;
        default:
            break;
        }
        const Result result = convertSendSocketErrorToResult(static_cast<int>(error));
        return result;
    }

    if (sentSize != 0)
        *sentSize = sent;
    return 0;
}

Result Socket::SendToMulti(const void* buffer, int size, const SockAddrIn* addresses, int count, int* sentSize)
{
    const int handle = m_handle;
    if (handle < 0)
        return cResultInvalidArgument;
    if (count < 1 || count > cMulticastCountMax)
        return cResultInvalidArgument;

    const int sent = sendto_multi(handle, buffer, size, 0, addresses, count);
    if (sent < 0)
    {
        const unsigned int error = GetSocketLastError();
        switch (error)
        {
        case cErrorClosed:
        case cErrorConnectionReset:
            if (m_handle < 0)
                return cResultDisconnected;
            if (handle != m_handle)
                return cResultBufferFull;
            return cResultDisconnected;
        case cErrorSendClosed:
            return cResultDisconnected;
        case cErrorSendAborted:
        case 0x2d:
        case 0x2e:
        case 0x2f:
            return cResultDisconnected;
        case cErrorNoBuffer:
            return cResultBufferFull;
        default:
            break;
        }
        const Result result = convertSendSocketErrorToResult(static_cast<int>(error));
        return result;
    }

    if (sentSize != 0)
        *sentSize = sent;
    return 0;
}

Result Socket::SetTtl(unsigned char ttl)
{
    const int handle = m_handle;
    if (handle < 0)
        return cResultInvalidArgument;

    int value = ttl;
    if (setsockopt(handle, cLevelIp, cOptionIpTtl, &value, sizeof(value)) < 0)
    {
        const int error = GetSocketLastError();
        if (IsClosedSocketError(error))
        {
            if (m_handle < 0)
                return cResultDisconnected;
            if (handle == m_handle)
                return cResultDisconnected;
        }
        return convertCommonSocketErrorToResult(error);
    }
    return 0;
}

Result Socket::InetAtoH(const char* text, InetAddress* address)
{
    if (text == 0 || address == 0)
        return ResultInvalidArgument();

    unsigned int value[2];
    if (inet_aton(text, value) != 0)
    {
        address->m_address = value[0];
        return 0;
    }
    return cResultInvalidArgument;
}

}
}
}
