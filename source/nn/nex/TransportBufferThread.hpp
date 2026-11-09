#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTBUFFERTHREAD_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTBUFFERTHREAD_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/ObjectThread.hpp"
#include "nn/nex/PseudoSingleton.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/TransportBuffer.hpp"
#include "nn/nex/TransportBufferThreadInterface.hpp"
#include "nn/nex/qSortedVector.hpp"

namespace nn
{
namespace nex
{

class QueuingSocket;

const unsigned int TRANSPORT_BUFFER_THREAD_TRACE_MASK = 0x8000;

class TransportBufferThread : public TransportBufferThreadInterface
{
public:
    TransportBufferThread()
        : m_bStopped(false),
          m_uiContextID(g_usingInstantiationContext ? g_instantiationContextVariable->GetRef() : 0),
          m_criticalSection(TRANSPORT_BUFFER_THREAD_TRACE_MASK),
          m_thread(String(L"Transport Buffer Thread"), g_transportBufferThreadPriority)
    {
        if (g_transportBufferThreadAffinity != INHERIT_THREAD_AFFINITY)
            m_thread.SetAffinity(g_transportBufferThreadAffinity);
    }

    virtual ~TransportBufferThread();

    virtual void Start();
    virtual void Stop();
    virtual bool InsertSocketPort(unsigned short uiPort, QueuingSocket* pSocket);
    virtual void EraseSocketPort(unsigned short uiPort);
    virtual void SetPriority(unsigned int uiPriority);
    virtual unsigned int GetPriority() const;

    void TransportLoop(void* pParameter);

private:
    bool m_bStopped;
    unsigned int m_uiContextID;
    ForcedCriticalSection m_criticalSection;
    qSortedVector<unsigned short, QueuingSocket*> m_sockets;
    ObjectThread<TransportBufferThread, void*> m_thread;
};

}
}

#endif
