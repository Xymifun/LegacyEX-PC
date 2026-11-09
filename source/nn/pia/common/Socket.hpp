#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SOCKET_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SOCKET_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/InetAddress.hpp"

namespace nn
{
namespace pia
{
namespace common
{

struct SockAddrIn;

int GetSocketLastError();
Result convertCommonSocketErrorToResult(int error);
Result convertRecvSocketErrorToResult(int error);
Result convertSendSocketErrorToResult(int error);

class Socket
{
public:
    Socket();
    ~Socket();
    Result Open(int type, int protocol);
    Result Open();
    Result Close();
    Result SetReuseAddress(bool enabled);
    Result Bind(const InetAddress& address, bool reuseAddress);
    Result Bind(unsigned short port, bool reuseAddress);
    Result RecvFrom(unsigned char* buffer, int size, InetAddress* address, unsigned char* ancillary, int* receivedSize);
    Result SendTo(const void* buffer, int size, const InetAddress& address, int* sentSize);
    Result SendToMulti(const void* buffer, int size, const SockAddrIn* addresses, int count, int* sentSize);
    Result SetTtl(unsigned char ttl);
    static Result InetAtoH(const char* text, InetAddress* address);
    void SetBroadcast(bool enabled);
    static void InetHtoA(const InetAddress& address, char* buffer, unsigned int size);

    int m_handle;
    int m_openCount;
    InetAddress m_address;
};

}
}
}

#endif
