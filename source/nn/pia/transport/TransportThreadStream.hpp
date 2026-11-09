#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_TRANSPORT_THREAD_STREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_TRANSPORT_THREAD_STREAM_HPP

#include "nn/pia/transport/PacketStream.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class LatencyEmulator;

class TransportThreadStream : public PacketStream
{
public:
    enum ThreadState
    {
        cThreadStateStop = 1,
        cThreadStateRun = 2,
        cThreadStateError = 3,
        cThreadStateQuit = 4,
        cThreadStateRequest = 0x10
    };

    TransportThreadStream();
    Result InitializeCore(const char* name, int priority, unsigned int packetNum, unsigned int latencyEntryNum,
                          bool isPacketLossEnabled, unsigned int streamIndex);
    void FinalizeCore();
    Result Startup();
    void Cleanup();
    Result GetLastResult() const;
    void ClearLastResult();
    void SetLatency(int minimumLatency, int maximumLatency);
    void SetPacketLossRatio(int packetLossRatio);
    void ThreadFunc(pead::Thread* thread, int message);
    void Trace(unsigned long long value) const;

    virtual void OnStartup() {}
    virtual Result ProcessOne() = 0;
    virtual ~TransportThreadStream();

    pead::Delegate2<TransportThreadStream, pead::Thread*, int> m_threadDelegate;
    pead::Thread* m_thread;
    pead::Event m_event;
    ThreadState m_threadState;
    int m_sleepMilliSecond;
    Result m_lastResult;
    LatencyEmulator* m_latencyEmulator;
    int m_packetLossRatio;
    pead::Random m_random;
    bool m_isPacketLossEnabled;

protected:
    void ChangeState(ThreadState threadState);
    bool isDropPacket();
};

}
}
}

#endif
