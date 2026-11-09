#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_TRANSPORT_ANALYSIS_DATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_TRANSPORT_ANALYSIS_DATA_HPP

#include "nn/pia/transport/ConnectionAnalysisData.hpp"
#include "nn/pia/transport/PacketAnalysisData.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class TransportAnalysisData
{
public:
    void Print(bool isDetail, bool skipIntermediate) const;
    void Trace(unsigned long long value) const;

    PacketAnalysisData m_packetAnalysisData;
    PacketAnalysisData m_receivedPacketAnalysisData;
    PacketAnalysisData m_sentPacketAnalysisData;
    PacketAnalysisData m_retransmittedPacketAnalysisData;
    ConnectionAnalysisData m_connectionAnalysisData;
    unsigned int m_dispatchCount;
};

}
}
}

#endif
