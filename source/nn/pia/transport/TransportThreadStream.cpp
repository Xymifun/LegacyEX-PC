#include "nn/pia/transport/TransportThreadStream.hpp"

#include "nn/pia/transport/LatencyEmulator.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include <new>

namespace nn
{
namespace pia
{
namespace transport
{

TransportThreadStream::TransportThreadStream()
    : PacketStream(),
      m_threadDelegate(this, &TransportThreadStream::ThreadFunc),
      m_thread(0),
      m_event(false),
      m_threadState(cThreadStateQuit),
      m_sleepMilliSecond(5),
      m_lastResult(0),
      m_latencyEmulator(0),
      m_packetLossRatio(0),
      m_isPacketLossEnabled(false)
{
}

TransportThreadStream::~TransportThreadStream()
{
}

Result TransportThreadStream::InitializeCore(const char* name, int priority, unsigned int packetNum,
                                             unsigned int latencyEntryNum, bool isPacketLossEnabled,
                                             unsigned int streamIndex)
{
    if (m_threadState != cThreadStateQuit)
        return cResultInvalidState;

    if (latencyEntryNum == 0)
    {
        m_latencyEmulator = 0;
    }
    else
    {
        void* storage = pead::allocateFromSeadHeap(sizeof(LatencyEmulator), common::HeapManager::GetHeap());
        m_latencyEmulator = storage != 0 ? new (storage) LatencyEmulator(latencyEntryNum) : 0;
    }
    m_isPacketLossEnabled = isPacketLossEnabled;
    pead::Heap* heap = common::HeapManager::GetHeap();
    void* storage = pead::allocateFromSeadHeap(sizeof(pead::DelegateThread), common::HeapManager::GetHeap());
    m_thread = storage != 0 ? new (storage)
                                  pead::DelegateThread(pead::SafeStringBase<char>(name), &m_threadDelegate, heap,
                                                       priority, pead::MessageQueue::cNoBlock, 0x7fffffff, 0x2000, 0x20)
                            : 0;
    m_thread->setAffinity(pead::CoreIdMask(1 << pead::CoreInfo::getCurrentCoreId().value()));
    PacketStream::Initialize(packetNum, streamIndex);
    m_threadState = cThreadStateStop;
    m_lastResult = 0;
    m_thread->start();
    return 0;
}

void TransportThreadStream::FinalizeCore()
{
    if (m_thread != 0)
    {
        ChangeState(cThreadStateQuit);
        m_thread->waitDone();
        if (m_thread != 0)
        {
            pead::Thread* thread = m_thread;
            thread->~Thread();
            pead::freeToSeadHeap(thread);
        }
        m_thread = 0;
    }
    PacketStream::Finalize();
    m_lastResult = 0;
    if (m_latencyEmulator != 0)
    {
        LatencyEmulator* latencyEmulator = m_latencyEmulator;
        latencyEmulator->~LatencyEmulator();
        pead::freeToSeadHeap(latencyEmulator);
        m_latencyEmulator = 0;
    }
}

void TransportThreadStream::ChangeState(ThreadState threadState)
{
    if (m_threadState == threadState)
        return;

    pead::TickSpan interval = pead::TickSpan::makeFromMicroSeconds(100);

    do
    {
        m_threadState = static_cast<ThreadState>(threadState | cThreadStateRequest);
        m_event.setSignal();
        while (m_threadState != threadState)
        {
            pead::Thread::sleep(interval);
            if (m_threadState == cThreadStateError)
                break;
        }
        if (m_threadState == threadState)
            return;
    } while (threadState != cThreadStateRun);
}

Result TransportThreadStream::Startup()
{
    if (m_threadState != cThreadStateStop)
        return cResultInvalidState;

    if (m_latencyEmulator != 0)
        m_latencyEmulator->Clear();

    PacketStream::Startup();
    m_lastResult = 0;
    OnStartup();
    ChangeState(cThreadStateRun);
    return 0;
}

void TransportThreadStream::Cleanup()
{
    if (m_threadState == cThreadStateQuit)
        return;

    ChangeState(cThreadStateStop);
    PacketStream::Cleanup();
    if (m_latencyEmulator != 0)
        m_latencyEmulator->Clear();
}

Result TransportThreadStream::GetLastResult() const
{
    switch (m_threadState)
    {
    case cThreadStateStop:
    case cThreadStateError:
        return m_lastResult;
    default:
        return 0;
    }
}

void TransportThreadStream::SetLatency(int minimumLatency, int maximumLatency)
{
    if (m_latencyEmulator != 0)
        m_latencyEmulator->SetLatency(minimumLatency, maximumLatency);
}

void TransportThreadStream::SetPacketLossRatio(int packetLossRatio)
{
    m_packetLossRatio = packetLossRatio;
}

void TransportThreadStream::ThreadFunc(pead::Thread* thread, int)
{
    for (;;)
    {
        switch (m_threadState)
        {
        case cThreadStateStop | cThreadStateRequest:
            m_threadState = cThreadStateStop;
            break;
        case cThreadStateRun | cThreadStateRequest:
            m_threadState = cThreadStateRun;
            break;
        case cThreadStateQuit | cThreadStateRequest:
            thread->quit(false);
            m_threadState = cThreadStateQuit;
            return;
        case cThreadStateRun:
        {
            Result result = ProcessOne();
            if (m_latencyEmulator != 0)
            {
                Result dispatchResult = m_latencyEmulator->Dispatch();
                if (dispatchResult != 0)
                    result = dispatchResult;
            }
            if (result == 0 || result == cResultDataSizeInvalid)
                break;

            if (result == cResultWindowOverflow)
            {
                m_event.wait(pead::TickSpan::makeFromMilliSeconds(m_sleepMilliSecond));
            }
            else if (result == cResultDataNotReady)
            {
                m_event.wait(pead::TickSpan::makeFromMilliSeconds(m_sleepMilliSecond));
            }
            else
            {
                m_lastResult = result;
                m_threadState = cThreadStateError;
            }
            break;
        }
        case cThreadStateStop:
        case cThreadStateError:
            m_event.wait();
            break;
        default:
            break;
        }
    }
}

bool TransportThreadStream::isDropPacket()
{
    if (m_isPacketLossEnabled)
    {
        int packetLossRatio = m_packetLossRatio;
        if (packetLossRatio != 0 && static_cast<int>(m_random.getU32(100)) < packetLossRatio)
            return true;
    }
    return false;
}

}
}
}
