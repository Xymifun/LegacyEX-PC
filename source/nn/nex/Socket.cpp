#include "nn/nex/Socket.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/IOCompletionContext.hpp"
#include "nn/nex/StringStream.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned char DEFAULT_TTL = 0x40;

}

const unsigned int SOCKET_SUCCESS = 0;

Socket::Socket(unsigned int uiTrafficType, unsigned int uiMTU) : m_uiState(STATE_CLOSED)
{
    m_uiTrafficType = uiTrafficType;
    m_bConnected = true;
    m_bVDP = false;
    m_bUDP = true;
    m_uiMTU = uiMTU;
    m_pDriverSocket = g_socketDriver != 0 ? g_socketDriver->Create() : 0;
}

Socket::~Socket()
{
    Close();
    if (g_socketDriver != 0)
        g_socketDriver->Delete(m_pDriverSocket);
}

InetAddress* Socket::GetAddress()
{
    return &m_address;
}

void Socket::Close()
{
    if (m_uiState != STATE_CLOSED && m_pDriverSocket != 0)
    {
        m_pDriverSocket->Close();
        m_uiState = STATE_CLOSED;
    }
}

bool Socket::Open(bool bUDP)
{
    m_bUDP = bUDP;
    if (m_pDriverSocket != 0
        && m_pDriverSocket->Open(bUDP ? SocketDriver::TRAFFIC_DATAGRAM : SocketDriver::TRAFFIC_STREAM))
    {
        m_uiState = STATE_OPEN;
        return true;
    }

    return false;
}

void Socket::Shutdown()
{
    if (m_uiState == STATE_CLOSED)
        return;

    if (!m_bConnected)
        return;

    if (m_pDriverSocket == 0)
        return;

    m_pDriverSocket->Shutdown();
}

void Socket::SetTTL(unsigned char uiTTL)
{
    m_pDriverSocket->SetTTL(uiTTL);
}

bool Socket::Bind(InetAddress* pAddress, bool bReuse)
{
    if (m_pDriverSocket != 0)
    {
        unsigned short uiPort = pAddress->GetPortNumber();
        if (m_pDriverSocket->Bind(uiPort))
        {
            pAddress->SetPortNumber(uiPort);
            SetTTL(DEFAULT_TTL);
            m_uiState = STATE_BOUND;
            m_address = *pAddress;
            return true;
        }
    }

    return false;
}

bool Socket::ProcessDriverSocketResult(SocketDriver::_Result result, IOCompletionContext* pContext)
{
    switch (result)
    {
    case SocketDriver::RESULT_CLOSED:
        if (m_bConnected)
            m_bConnected = false;
        break;
    case SocketDriver::RESULT_SUCCESS:
        return true;
    }

    return false;
}

int Socket::RecvImpl(unsigned char* pBuffer, unsigned int uiSize, unsigned int* pReceived, InetAddress* pAddress,
                     SocketDriver::_SocketFlag flag)
{
    if (m_pDriverSocket == 0)
        return -1;

    SocketDriver::_Result result;
    unsigned int uiReceived = 0;
    if (m_bUDP)
    {
        SocketDriver::InetAddress from;
        result = m_pDriverSocket->RecvFrom(pBuffer, uiSize, &from, &uiReceived, flag);
        pAddress->SetAddress(from.m_uiAddress);
        pAddress->SetPortNumber(from.m_uiPort);
    }
    else
    {
        result = m_pDriverSocket->Recv(pBuffer, uiSize, &uiReceived);
        pAddress->SetAddress(0U);
        pAddress->SetPortNumber(0);
    }

    if (!ProcessDriverSocketResult(result, 0))
        return -1;

    *pReceived = uiReceived;
    return 0;
}

int Socket::RecvNonBlock(unsigned char* pBuffer, unsigned int uiSize, unsigned int* pReceived, InetAddress* pAddress)
{
    return RecvImpl(pBuffer, uiSize, pReceived, pAddress, SocketDriver::SOCKET_FLAG_NON_BLOCK);
}

int Socket::Recv(Buffer* pBuffer, InetAddress* pAddress)
{
    unsigned int uiReceived = 0;
    int iResult = RecvImpl(pBuffer->GetData(), pBuffer->GetRealSize() - pBuffer->GetHeadShiftSize(), &uiReceived,
                           pAddress, SocketDriver::SOCKET_FLAG_NONE);
    if (iResult == 0)
        pBuffer->SetLength(uiReceived);

    return iResult;
}

int Socket::RecvNonBlock(Buffer* pBuffer, InetAddress* pAddress)
{
    unsigned int uiReceived = 0;
    int iResult = RecvImpl(pBuffer->GetData(), pBuffer->GetRealSize() - pBuffer->GetHeadShiftSize(), &uiReceived,
                           pAddress, SocketDriver::SOCKET_FLAG_NON_BLOCK);
    if (iResult == 0)
        pBuffer->SetLength(uiReceived);

    return iResult;
}

int Socket::Send(const unsigned char* pBuffer, unsigned int uiSize, const InetAddress& rAddress)
{
    if (!m_bConnected)
    {
        ProcessDriverSocketResult(SocketDriver::RESULT_CLOSED, 0);
        return -1;
    }

    if (m_pDriverSocket != 0)
    {
        SocketDriver::_Result result;
        unsigned int uiSent = 0;
        if (m_bUDP)
        {
            SocketDriver::InetAddress to(rAddress.GetAddress(), rAddress.GetPortNumber());
            result = m_pDriverSocket->SendTo(const_cast<unsigned char*>(pBuffer), uiSize, to, &uiSent);
        }
        else
        {
            result = m_pDriverSocket->Send(const_cast<unsigned char*>(pBuffer), uiSize, &uiSent);
        }

        if (ProcessDriverSocketResult(result, 0))
            return 0;
    }

    return -1;
}

int Socket::Send(const Buffer& rBuffer, const InetAddress& rAddress)
{
    unsigned char uiTTL = rBuffer.GetTTL();
    if (uiTTL != 0)
        SetTTL(uiTTL);

    int iResult = Send(rBuffer.GetData(), rBuffer.GetLength(), rAddress);
    if (uiTTL != 0)
        SetTTL(DEFAULT_TTL);

    return iResult;
}

void Socket::Poll(PollEvent events, PollEvent* pResult, unsigned int uiTimeout)
{
    SocketDriver::PollInfo info[1];
    info[0].m_pSocket = m_pDriverSocket;
    info[0].m_uiEvents = events;
    info[0].m_uiResult = 0;
    bool bResult = g_socketDriver->Poll(info, 1, uiTimeout);
    if (bResult)
        *pResult = static_cast<PollEvent>(info[0].m_uiResult);
}

bool Socket::Connect(InetAddress* pAddress, IOCompletionContext* pContext)
{
    pContext->m_uiOperation = IOCompletionContext::OPERATION_CONNECT;
    pContext->m_address = *pAddress;
    pContext->m_pSocket = this;
    if (m_pDriverSocket != 0)
    {
        SocketDriver::InetAddress to(pContext->m_address.GetAddress(), pContext->m_address.GetPortNumber());
        SocketDriver::_Result result = m_pDriverSocket->Connect(to);
        return ProcessDriverSocketResult(result, pContext);
    }

    return false;
}

bool Socket::SetMulticastAddress(unsigned int uiAddress)
{
    if (m_pDriverSocket != 0)
        return m_pDriverSocket->SetMulticastAddress(uiAddress);

    return false;
}

void Socket::Trace(unsigned long long uiFilter)
{
    StringStream stream;
    operator<<(stream, *this);
}

StringStream& operator<<(StringStream& rStream, Socket& rSocket)
{
    StringStream& rState = operator<<(rStream << L" Addr=", rSocket.m_address) << L" State=";
    StringStream& rLength = rState.StreamNumber(rSocket.m_uiState) << L" RecvAddrLen=";
    StringStream& rUDP = rLength.StreamNumber(rSocket.m_iRecvAddrLen) << L" UDP? ";
    StringStream& rVDP = (rUDP << rSocket.m_bUDP) << L" VDP? ";
    StringStream& rTraffic = (rVDP << rSocket.m_bVDP) << L" TrafficType=";
    (rTraffic.StreamNumber(rSocket.m_uiTrafficType) << L" MTU=").StreamNumber(rSocket.m_uiMTU);
    return rStream;
}

}
}
