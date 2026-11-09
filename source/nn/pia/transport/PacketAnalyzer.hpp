#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PACKET_ANALYZER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PACKET_ANALYZER_HPP

#include "nn/pia.h"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/transport/PacketAnalysisData.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class Packet;
}
}
}

namespace nn
{
namespace pia
{
namespace transport
{

class PacketAnalyzer : public PacketAnalysisData
{
public:
    PacketAnalyzer(const char* name, unsigned int headerSize);
    ~PacketAnalyzer();
    Result Startup();
    void Cleanup();
    Result UpdatePacketAnalysisData(const common::Packet& packet, bool isRelay, bool isSingleCount);
    void ClearPacketAnalysisData();
    Result GetPacketAnalysisData(PacketAnalysisData* packetAnalysisData) const;
    void Trace(unsigned long long value) const;

    mutable common::CriticalSection m_criticalSection;
    common::Time m_startTime;
    bool m_startedUp;
    unsigned int m_headerSize;
};

}
}
}

#endif
