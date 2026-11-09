#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTBUFFERMULTITHREAD_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TRANSPORTBUFFERMULTITHREAD_HPP

#include <utility>
#include <vector>

#include "nn/nex/Condition.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/ObjectThread.hpp"
#include "nn/nex/TransportBufferThreadInterface.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class QueuingSocket;

class TransportBufferMultiThread : public TransportBufferThreadInterface
{
public:
    enum
    {
        RECV_THREAD_COUNT = 3
    };

    struct PortThreadSocket
    {
        unsigned short m_uiPort;
        int m_iThread;
        QueuingSocket* m_pSocket;
    };

    typedef std::pair<unsigned short, QueuingSocket*> PortSocket;
    typedef ObjectThread<TransportBufferMultiThread, int> Thread;

    TransportBufferMultiThread();
    virtual ~TransportBufferMultiThread();

    virtual void Start();
    virtual void Stop();
    virtual bool InsertSocketPort(unsigned short uiPort, QueuingSocket* pSocket);
    virtual void EraseSocketPort(unsigned short uiPort);
    virtual void SetPriority(unsigned int uiPriority);
    virtual unsigned int GetPriority() const;

    void TransportSendLoop(int iThread);
    void TransportRecvLoop(int iThread);

    Condition* GetCondition() { return &m_condition; }

private:
    bool m_bStopped;
    bool m_bBusy;
    unsigned int m_uiContextID;
    ForcedCriticalSection m_criticalSection;
    bool m_bSlotUsed[RECV_THREAD_COUNT];
    volatile bool m_bSlotErasing[RECV_THREAD_COUNT];
    int m_iErasingPort;
    qVector<PortThreadSocket> m_portThreads;
    ForcedCriticalSection* m_pSlotLocks[RECV_THREAD_COUNT];
    PortSocket m_slots[RECV_THREAD_COUNT];
    qVector<PortSocket> m_sendSockets;
    ForcedCriticalSection m_sendCriticalSection;
    Thread* m_pRecvThreads[RECV_THREAD_COUNT];
    Condition m_condition;
    ForcedCriticalSection m_conditionCriticalSection;
    Thread* m_pSendThread;
};

}
}

#endif
