#include "nn/nex/TransportBufferMultiThread.hpp"

#include <coreinit/thread.h>

#include "nn/nex/Platform.hpp"
#include "nn/nex/PseudoSingleton.hpp"
#include "nn/nex/QueuingSocketTransportBuffer.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/TransportBuffer.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int CRITICAL_SECTION_TRACE_MASK = 0x8000;
const unsigned int LOCK_TRACE_MASK = 0x10000;
const unsigned int POLL_SLEEP_MILLISECONDS = 2;
const unsigned int INFINITE_TIMEOUT = 0xFFFFFFFF;

}

TransportBufferMultiThread::TransportBufferMultiThread()
    : m_bStopped(false),
      m_bBusy(false),
      m_uiContextID(g_usingInstantiationContext ? g_instantiationContextVariable->GetRef() : 0),
      m_criticalSection(CRITICAL_SECTION_TRACE_MASK),
      m_iErasingPort(-1),
      m_sendCriticalSection(LOCK_TRACE_MASK),
      m_condition(LOCK_TRACE_MASK, false),
      m_conditionCriticalSection(LOCK_TRACE_MASK)
{
    m_portThreads.reserve(RECV_THREAD_COUNT);
    m_sendSockets.reserve(RECV_THREAD_COUNT);
    for (int i = 0; i < RECV_THREAD_COUNT; ++i)
    {
        m_pRecvThreads[i] = 0;
        m_pSlotLocks[i] = new ForcedCriticalSection(LOCK_TRACE_MASK);
        m_bSlotUsed[i] = false;
        m_bSlotErasing[i] = false;
        m_slots[i].second = 0;
        m_slots[i].first = 0;
    }
    m_pSendThread = new Thread(String(L"Transport Buffer Thread (send)"), g_transportBufferSendThreadPriority);
    if (g_transportBufferThreadAffinity != INHERIT_THREAD_AFFINITY)
        m_pSendThread->SetAffinity(g_transportBufferThreadAffinity);
}

TransportBufferMultiThread::~TransportBufferMultiThread()
{
    Stop();
    for (int i = 0; i < RECV_THREAD_COUNT; ++i)
    {
        delete m_pRecvThreads[i];
        delete m_pSlotLocks[i];
    }
    delete m_pSendThread;
}

void TransportBufferMultiThread::Start()
{
    Thread* pThread = m_pSendThread;
    pThread->SetObjectMethod(&TransportBufferMultiThread::TransportSendLoop, this, 0);
    pThread->Launch(0);
}

void TransportBufferMultiThread::Stop()
{
    if (!m_bStopped)
    {
        m_bStopped = true;
        int iCount = static_cast<int>(m_portThreads.size());
        unsigned short aPorts[RECV_THREAD_COUNT];
        for (int i = 0; i < iCount; ++i)
            aPorts[i] = m_portThreads[i].m_uiPort;
        for (int i = 0; i < iCount; ++i)
            EraseSocketPort(aPorts[i]);
        m_condition.Notify();
        if (m_pSendThread->IsLaunched())
            m_pSendThread->Wait(INFINITE_TIMEOUT);
        for (int i = 0; i < RECV_THREAD_COUNT; ++i)
        {
            if (m_pRecvThreads[i] != 0 && m_pRecvThreads[i]->IsLaunched())
                m_pRecvThreads[i]->Wait(INFINITE_TIMEOUT);
        }
    }
}

bool TransportBufferMultiThread::InsertSocketPort(unsigned short uiPort, QueuingSocket* pSocket)
{
    CriticalSection* pLock = &m_criticalSection;
    pLock->Enter();
    int iSlot = -1;
    for (int i = 0; i < RECV_THREAD_COUNT; ++i)
    {
        if (!m_bSlotUsed[i])
            iSlot = i;
    }
    if (iSlot == -1)
    {
        pLock->Leave();
        return false;
    }
    if (m_pRecvThreads[iSlot] == 0)
    {
        m_pRecvThreads[iSlot] = new Thread(String(L"Transport Buffer Thread (recv)"), g_transportBufferThreadPriority);
        if (g_transportBufferThreadAffinity != INHERIT_THREAD_AFFINITY)
            m_pRecvThreads[iSlot]->SetAffinity(g_transportBufferThreadAffinity);
        m_pRecvThreads[iSlot]->SetObjectMethod(&TransportBufferMultiThread::TransportRecvLoop, this, iSlot);
        m_pRecvThreads[iSlot]->Launch(0);
    }
    PortThreadSocket portThread;
    portThread.m_uiPort = uiPort;
    portThread.m_iThread = iSlot;
    portThread.m_pSocket = pSocket;
    m_portThreads.push_back(portThread);
    m_bSlotUsed[iSlot] = true;
    m_bBusy = true;
    m_pSlotLocks[iSlot]->Enter();
    m_slots[iSlot].second = pSocket;
    m_slots[iSlot].first = uiPort;
    m_pSlotLocks[iSlot]->Leave();
    m_sendCriticalSection.Enter();
    m_sendSockets.push_back(PortSocket(uiPort, pSocket));
    m_sendCriticalSection.Leave();
    m_bBusy = false;
    pLock->Leave();
    return true;
}

void TransportBufferMultiThread::EraseSocketPort(unsigned short uiPort)
{
    CriticalSection* pLock = &m_criticalSection;
    pLock->Enter();
    qVector<PortThreadSocket>::iterator end = m_portThreads.end();
    qVector<PortThreadSocket>::iterator portThread;
    for (portThread = m_portThreads.begin(); portThread != end; ++portThread)
    {
        if (portThread->m_uiPort == uiPort)
            break;
    }
    if (portThread == end)
    {
        pLock->Leave();
        return;
    }
    int iSlot = portThread->m_iThread;
    QueuingSocket* pSocket = portThread->m_pSocket;
    m_bSlotErasing[iSlot] = true;
    m_iErasingPort = uiPort;
    pSocket->Shutdown();
    m_portThreads.erase(portThread);
    m_bSlotUsed[iSlot] = false;
    m_bBusy = true;
    m_pSlotLocks[iSlot]->Enter();
    m_slots[iSlot].first = 0;
    m_slots[iSlot].second = 0;
    m_bSlotErasing[iSlot] = false;
    m_pSlotLocks[iSlot]->Leave();
    m_sendCriticalSection.Enter();
    qVector<PortSocket>::iterator sendSocket = m_sendSockets.begin();
    qVector<PortSocket>::iterator sendEnd = m_sendSockets.end();
    while (sendSocket != sendEnd && sendSocket->first != uiPort)
        ++sendSocket;
    m_sendSockets.erase(sendSocket);
    m_iErasingPort = -1;
    m_sendCriticalSection.Leave();
    m_bBusy = false;
    pLock->Leave();
}

void TransportBufferMultiThread::SetPriority(unsigned int uiPriority)
{
    for (int i = 0; i < RECV_THREAD_COUNT; ++i)
    {
        if (m_pRecvThreads[i] != 0)
            m_pRecvThreads[i]->SetPriority(uiPriority);
    }
    m_pSendThread->SetPriority(uiPriority);
}

unsigned int TransportBufferMultiThread::GetPriority() const
{
    return m_pSendThread->GetPriority();
}

void TransportBufferMultiThread::TransportSendLoop(int iThread)
{
    PseudoSingleton::SetContextIfRequired(m_uiContextID);
    OSGetCurrentThread();
    while (!m_bStopped)
    {
        bool bSent = false;
        m_sendCriticalSection.Enter();
        for (qVector<PortSocket>::iterator socket = m_sendSockets.begin(); socket != m_sendSockets.end(); ++socket)
        {
            QueuingSocketTransportBuffer* pSocket = static_cast<QueuingSocketTransportBuffer*>(socket->second);
            if (pSocket != 0 && socket->first != m_iErasingPort && pSocket->SendBufferCore())
                bSent = true;
        }
        m_sendCriticalSection.Leave();
        if (m_bBusy)
            Platform::Sleep(POLL_SLEEP_MILLISECONDS);
        if (!bSent)
        {
            m_conditionCriticalSection.Enter();
            m_condition.WaitForEvent(&m_conditionCriticalSection, INFINITE_TIMEOUT);
            m_conditionCriticalSection.Leave();
        }
    }
}

void TransportBufferMultiThread::TransportRecvLoop(int iThread)
{
    PseudoSingleton::SetContextIfRequired(m_uiContextID);
    OSGetCurrentThread();
    while (!m_bStopped)
    {
        bool bReceived = false;
        if (!m_bSlotErasing[iThread])
        {
            m_pSlotLocks[iThread]->Enter();
            QueuingSocketTransportBuffer* pSocket = static_cast<QueuingSocketTransportBuffer*>(m_slots[iThread].second);
            if (pSocket != 0)
                bReceived = pSocket->RecvCore(false);
            m_pSlotLocks[iThread]->Leave();
        }
        if (!bReceived || m_bBusy)
            Platform::Sleep(POLL_SLEEP_MILLISECONDS);
    }
}

}
}
