#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_SEND_THREAD_STREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_SEND_THREAD_STREAM_HPP

#include "nn/pia/transport/TransportThreadStream.hpp"

namespace nn
{
namespace pia
{
namespace common
{
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

class SendThreadStream : public TransportThreadStream
{
public:
    SendThreadStream();
    virtual ~SendThreadStream();
    Result Initialize(common::IPacketOutput* packetOutput, unsigned int packetNum, int priority,
                      unsigned int latencyEntryNum, bool isPacketLossEnabled);
    void Finalize();
    void SetMonitoringData();
    void Trace(unsigned long long value) const;

    virtual Result ProcessOne();

    common::IPacketOutput* m_packetOutput;
    unsigned int m_sendPacketNum;
    unsigned long long m_sendDataSize;
};

}
}
}

#endif
