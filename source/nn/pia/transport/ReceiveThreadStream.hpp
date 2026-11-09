#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RECEIVE_THREAD_STREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RECEIVE_THREAD_STREAM_HPP

#include "nn/pia/transport/TransportThreadStream.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class IPacketInput;
}
}
}

namespace nn
{
namespace pia
{
namespace transport
{

class ReceiveThreadStream : public TransportThreadStream
{
public:
    ReceiveThreadStream();
    virtual ~ReceiveThreadStream();
    Result Initialize(common::IPacketInput* packetInput, unsigned int packetNum, int priority,
                      unsigned int latencyEntryNum, bool isPacketLossEnabled);
    void Finalize();
    void SetMonitoringData();
    void Trace(unsigned long long value) const;

    virtual void OnStartup();
    virtual Result ProcessOne();

    common::IPacketInput* m_packetInput;
    unsigned int m_receivePacketNum;
    unsigned long long m_receiveDataSize;
};

}
}
}

#endif
