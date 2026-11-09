#include "nn/nex/TransportBufferThread.hpp"

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
const unsigned int IDLE_SLEEP_MILLISECONDS = 100;

}

TransportBufferThread::~TransportBufferThread()
{
    Stop();
}

void TransportBufferThread::Start()
{
    m_bStopped = false;
    m_thread.SetObjectMethod(&TransportBufferThread::TransportLoop, this, 0);
    m_thread.Launch(0);
}

void TransportBufferThread::Stop()
{
    if (m_thread.IsLaunched())
    {
        m_bStopped = true;
        m_thread.Wait(0xFFFFFFFF);
    }
}

bool TransportBufferThread::InsertSocketPort(unsigned short uiPort, QueuingSocket* pSocket)
{
    CriticalSection* pLock = &m_criticalSection;
    pLock->Enter();
    m_sockets.insert(std::make_pair(uiPort, pSocket));
    pLock->Leave();
    return true;
}

void TransportBufferThread::EraseSocketPort(unsigned short uiPort)
{
    CriticalSection* pLock = &m_criticalSection;
    pLock->Enter();
    m_sockets.erase(uiPort);
    pLock->Leave();
}

void TransportBufferThread::SetPriority(unsigned int uiPriority)
{
    m_thread.SetPriority(uiPriority);
}

unsigned int TransportBufferThread::GetPriority() const
{
    return m_thread.GetPriority();
}

void TransportBufferThread::TransportLoop(void* pParameter)
{
    PseudoSingleton::SetContextIfRequired(m_uiContextID);
    while (!m_bStopped)
    {
        if (m_sockets.empty())
        {
            Platform::Sleep(IDLE_SLEEP_MILLISECONDS);
            continue;
        }
        Platform::Sleep(g_transportBufferThreadSleep);
        CriticalSection* pLock = &m_criticalSection;
        pLock->Enter();
        for (qSortedVector<unsigned short, QueuingSocket*>::iterator socket = m_sockets.begin();
             socket != m_sockets.end(); ++socket)
        {
            QueuingSocketTransportBuffer* pTransportSocket = static_cast<QueuingSocketTransportBuffer*>(socket->second);
            pTransportSocket->RecvCore(true);
            pTransportSocket->SendBufferCore();
        }
        pLock->Leave();
    }
}

}
}
