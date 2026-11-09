#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_BERKELEYSOCKETDRIVER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_BERKELEYSOCKETDRIVER_HPP

#include <netinet/in.h>
#include <stdint.h>

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/PseudoSingleton.hpp"
#include "nn/nex/SocketDriver.hpp"

extern "C"
{
uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);
char* inet_ntoa(struct in_addr in);
int inet_pton(int af, const char* src, void* dst);
}

in_addr_t inet_addr(const char* cp);

class SendMultiParameter;

namespace nn
{
namespace nex
{

class BerkeleySocketDriver : public SocketDriver
{
public:
    enum
    {
        PEER_ADDRESS_CAPACITY = 15
    };

    class BerkeleySocket : public SocketDriver::Socket
    {
    public:
        BerkeleySocket();

        virtual bool Open(_TrafficType type);
        virtual void Close();
        virtual bool Bind(unsigned short& rPort);
        virtual _Result RecvFrom(unsigned char* pBuffer, unsigned int uiSize, SocketDriver::InetAddress* pAddress,
                                 unsigned int* pReceived, _SocketFlag flag);
        virtual _Result SendTo(unsigned char* pBuffer, unsigned int uiSize, const SocketDriver::InetAddress& rAddress,
                               unsigned int* pSent);
        virtual bool Shutdown();
        virtual _Result Connect(const SocketDriver::InetAddress& rAddress);
        virtual _Result Recv(unsigned char* pBuffer, unsigned int uiSize, unsigned int* pReceived);
        virtual _Result Send(unsigned char* pBuffer, unsigned int uiSize, unsigned int* pSent);
        virtual bool SetMulticastAddress(unsigned int uiAddress);
        virtual bool SetTTL(unsigned char uiTTL);
        virtual ~BerkeleySocket();
        virtual bool SetAsync(bool bAsync);

        unsigned int GetLastSocketError(int iResult);
        _Result LastSocketErrorToResult(const wchar_t* pContext, int iResult);

    private:
        friend class BerkeleySocketDriver;

        int m_iHandle;
        SendMultiParameter* m_pSendMultiParameter;
    };

    virtual ~BerkeleySocketDriver();
    virtual Socket* Create();
    virtual void Delete(Socket* pSocket);
    virtual bool Poll(PollInfo* pInfo, unsigned int uiCount, unsigned int uiTimeout);

    static void GetPeerAddress(nn::nex::InetAddress* pAddresses, unsigned int uiCapacity, unsigned int* pCount);
    static void SetPeerAddress(const nn::nex::InetAddress* pAddresses, unsigned int uiCount);
    static void ResetAnalyzeInfo();
};

}
}

#endif
