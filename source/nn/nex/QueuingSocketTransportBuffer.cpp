#include "nn/nex/QueuingSocketTransportBuffer.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/Condition.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/TransportBuffer.hpp"

namespace nn
{
namespace nex
{

QueuingSocketTransportBuffer::QueuingSocketTransportBuffer(unsigned int uiTrafficType, RootTransport* pTransport,
                                                           unsigned int uiRecvCount, unsigned int uiSendCount,
                                                           unsigned int uiMTU, Condition* pCondition,
                                                           TransportBuffer* pTransportBuffer)
    : QueuingSocket(uiTrafficType, pTransport, uiMTU, pTransportBuffer),
      m_recvSet(pTransportBuffer, g_transportBufferCount),
      m_sendSet(pTransportBuffer, g_transportBufferCount)
{
    m_pCondition = pCondition;
    m_sendSet.m_uiBufferSize = m_uiMTU;
    m_recvSet.m_uiBufferSize = m_uiMTU;
    m_uiSendCount = uiSendCount;
    m_uiRecvCount = uiRecvCount;
    m_pRecvBuffer = new Buffer(m_uiMTU, 0);
}

QueuingSocketTransportBuffer::~QueuingSocketTransportBuffer()
{
    m_recvSet.ClearList_(m_recvSet.m_useInfos);
    m_sendSet.ClearList_(m_sendSet.m_useInfos);
    delete m_pRecvBuffer;
}

bool QueuingSocketTransportBuffer::IsRecvDataExists() const
{
    bool bEnabled = m_inputEmulator.GetDevice()->IsEnabled();
    if (bEnabled)
        return true;
    return m_recvSet.m_bHasUse;
}

int QueuingSocketTransportBuffer::RecvBuffer(Buffer* pBuffer, InetAddress* pAddress)
{
    TransportBufferInfo* pInfo = m_recvSet.PopNewUseInfo();
    if (pInfo == 0 || pInfo->m_pBuffer == 0)
        return -1;
    *pBuffer = *pInfo->m_pBuffer;
    *pAddress = pInfo->m_address;
    m_recvSet.FreeOneBuffer(pInfo);
    return 0;
}

int QueuingSocketTransportBuffer::SendBuffer(const Buffer& rBuffer, const InetAddress& rAddress)
{
    if (rBuffer.GetLength() != 0)
    {
        CriticalSection* pLock = &m_sendSet.m_criticalSection;
        pLock->Enter();
        TransportBufferInfo* pInfo = m_sendSet.PopNewSetInfo();
        bool bFailed = pInfo == 0 || pInfo->m_pBuffer == 0;
        if (bFailed)
        {
            if (m_pCondition != 0)
                m_pCondition->Notify();
            pLock->Leave();
            return -1;
        }
        *pInfo->m_pBuffer = rBuffer;
        pInfo->m_address = rAddress;
        if (m_pCondition != 0)
            m_pCondition->Notify();
        pLock->Leave();
        return 0;
    }
    return -1;
}

void QueuingSocketTransportBuffer::PushRecvBuffer(Buffer* pBuffer, const InetAddress& rAddress)
{
    CriticalSection* pLock = &m_recvSet.m_criticalSection;
    pLock->Enter();
    TransportBufferInfo* pInfo = m_recvSet.PopNewSetInfo();
    if (pInfo != 0 && pInfo->m_pBuffer != 0)
    {
        *pInfo->m_pBuffer = *pBuffer;
        pInfo->m_address = rAddress;
    }
    pLock->Leave();
}

bool QueuingSocketTransportBuffer::RecvCoreSingle(bool bNonBlock)
{
    bool bReceivable = !m_recvSet.m_bFull;
    if (bReceivable && m_bConnected)
    {
        InetAddress address;
        m_pRecvBuffer->Clear();
        int iResult;
        if (bNonBlock)
            iResult = Socket::RecvNonBlock(m_pRecvBuffer, &address);
        else
            iResult = Socket::Recv(m_pRecvBuffer, &address);
        if (iResult == 0)
        {
            PushRecvBuffer(m_pRecvBuffer, address);
            return true;
        }
    }
    return false;
}

bool QueuingSocketTransportBuffer::RecvCore(bool bNonBlock)
{
    for (unsigned int i = 0; i < m_uiRecvCount; ++i)
    {
        if (!RecvCoreSingle(bNonBlock))
            return false;
    }
    return true;
}

bool QueuingSocketTransportBuffer::SendBufferCoreSingle()
{
    bool bSendable = m_sendSet.m_bHasUse && m_bConnected;
    if (!bSendable)
        return false;
    TransportBufferInfo* pInfo = m_sendSet.PopNewUseInfo();
    if (pInfo == 0 || pInfo->m_pBuffer == 0)
        return false;
    int iResult = Socket::Send(*pInfo->m_pBuffer, pInfo->m_address);
    m_sendSet.FreeOneBuffer(pInfo);
    return iResult == 0;
}

bool QueuingSocketTransportBuffer::SendBufferCore()
{
    for (unsigned int i = 0; i < m_uiSendCount; ++i)
    {
        if (!SendBufferCoreSingle())
            return false;
    }
    return true;
}

}
}
