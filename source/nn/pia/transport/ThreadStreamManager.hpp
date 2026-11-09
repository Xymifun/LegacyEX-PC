#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_THREAD_STREAM_MANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_THREAD_STREAM_MANAGER_HPP

#include "nn/pia.h"
#include "nn/pia/transport/ReceiveThreadStream.hpp"
#include "nn/pia/transport/SendThreadStream.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class IPacketInput;
class IPacketOutput;
}
}
}

namespace nn
{
namespace pia
{
namespace transport
{

class NetworkFactory;

class ThreadStreamManager
{
public:
    ThreadStreamManager(NetworkFactory* networkFactory, unsigned int sendPacketNum, unsigned int receivePacketNum,
                        unsigned int sendLatencyEntryNum, unsigned int receiveLatencyEntryNum,
                        bool isPacketLossEnabled);
    static Result CreateInstance(NetworkFactory* networkFactory, unsigned int sendPacketNum,
                                 unsigned int receivePacketNum, unsigned int sendLatencyEntryNum,
                                 unsigned int receiveLatencyEntryNum, bool isPacketLossEnabled);
    static void DestroyInstance();
    Result Startup();
    void Cleanup();
    void SetSendThreadPriority(int priority);
    int GetSendThreadPriority() const;
    void SetReceiveThreadPriority(int priority);
    int GetReceiveThreadPriority() const;
    void SetSendThreadSleepTimeSpan(unsigned int milliSecond);
    unsigned int GetSendThreadSleepTimeSpan() const;
    void SetReceiveThreadSleepTimeSpan(unsigned int milliSecond);
    unsigned int GetReceiveThreadSleepTimeSpan() const;
    void SetSendThreadLatencyEmulation(int minimumLatency, int maximumLatency);
    void SetReceiveThreadLatencyEmulation(int minimumLatency, int maximumLatency);
    unsigned int GetSendThreadSendPacketNum() const;
    void ClearSendThreadSendPacketNum();
    unsigned int GetReceiveThreadReceivePacketNum() const;
    void ClearReceiveThreadReceivePacketNum();
    unsigned long long GetSendThreadSendDataSize() const;
    void ClearSendThreadSendDataSize();
    unsigned long long GetReceiveThreadReceiveDataSize() const;
    void ClearReceiveThreadReceiveDataSize();
    void SetSendThreadPacketLossRatio(int packetLossRatio);
    int GetSendThreadPacketLossRatio() const;
    void SetReceiveThreadPacketLossRatio(int packetLossRatio);
    int GetReceiveThreadPacketLossRatio() const;
    void SetMonitoringData();

    common::IPacketInput* m_packetInput;
    common::IPacketOutput* m_packetOutput;
    ReceiveThreadStream* m_receiveThreadStream;
    SendThreadStream* m_sendThreadStream;

    virtual void Trace(unsigned long long value) const;
    virtual ~ThreadStreamManager();
};

extern ThreadStreamManager* g_piaThreadStreamManager;

}
}
}

#endif
