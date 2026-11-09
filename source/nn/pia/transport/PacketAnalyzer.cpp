#include "nn/pia/transport/PacketAnalyzer.hpp"

#include <string.h>

#include "nn/pia/common/Math.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/PayloadSizeManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/transport/ProtocolMessageReader.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

PacketAnalyzer::PacketAnalyzer(const char* name, unsigned int headerSize)
    : PacketAnalysisData(),
      m_criticalSection(-1),
      m_startedUp(false),
      m_headerSize(headerSize)
{
    const unsigned int length = common::minimum<unsigned int>(strlen(name), cNameLengthMax - 1);
    memcpy(m_name, name, length);
    m_name[length] = '\0';
}

PacketAnalyzer::~PacketAnalyzer()
{
}

}
}
}

namespace
{
nn::pia::common::Time getTime()
{
    if (nn::pia::transport::g_piaTransport != 0)
        return nn::pia::transport::g_piaTransport->m_time;
    nn::pia::common::Time time;
    time.SetNow();
    return time;
}
}

namespace nn
{
namespace pia
{
namespace transport
{

Result PacketAnalyzer::Startup()
{
    if (m_startedUp)
        return static_cast<Result>(0x10c08);

    m_criticalSection.Lock();
    ClearExceptName();
    m_criticalSection.Unlock();

    m_startTime = getTime();
    m_startedUp = true;
    return 0;
}

void PacketAnalyzer::Cleanup()
{
    if (!m_startedUp)
        return;
    m_startedUp = false;
}

Result PacketAnalyzer::UpdatePacketAnalysisData(const common::Packet& packet, bool isRelay, bool isSingleCount)
{
    if (!m_startedUp)
        return static_cast<Result>(0x10c08);
    if (packet.IsValid() == 0)
        return static_cast<Result>(0x10c07);

    m_criticalSection.Lock();
    unsigned int offset = 0;
    const unsigned int count = (isRelay && !isSingleCount) ? static_cast<unsigned int>(packet.GetPacketNumInNetwork())
                                                           : 1;
    while (offset < packet.m_size - common::Packet::cHeaderSize)
    {
        ProtocolMessageReader reader;
        reader.Attach(packet, offset);
        if (reader.m_message == 0)
            break;

        const unsigned int messageSize = ((reader.m_messageSize + 0x17U) & ~3U) + 4U;
        const ProtocolId protocolId(reader.m_header.m_protocolId);
        int index = GetIndex(protocolId);
        if (index == -1)
            index = CreateIndex(protocolId);
        if (index != -1)
        {
            m_entries[index].m_packetNum += count;
            m_entries[index].m_packetSize += count * messageSize;
            m_entries[index].m_totalPacketNum += count;
            m_entries[index].m_totalPacketSize += count * messageSize;
        }
        offset += messageSize;
    }

    const unsigned int signatureSize = common::g_payloadSizeManager->m_signatureSize;
    m_packetNum += count;
    m_totalPacketNum += count;
    m_packetSize += count * (packet.m_size + signatureSize + m_headerSize);
    m_totalPacketSize += count * (packet.m_size + signatureSize + m_headerSize);
    m_criticalSection.Unlock();
    return 0;
}

void PacketAnalyzer::ClearPacketAnalysisData()
{
    m_criticalSection.Lock();
    ClearCounters();
    m_criticalSection.Unlock();
}

Result PacketAnalyzer::GetPacketAnalysisData(PacketAnalysisData* packetAnalysisData) const
{
    if (packetAnalysisData == 0)
        return static_cast<Result>(0x10c07);

    m_criticalSection.Lock();
    *packetAnalysisData = *this;
    m_criticalSection.Unlock();

    const common::Time now = getTime();
    packetAnalysisData->m_passedMilliSecond = static_cast<unsigned int>((now - m_startTime).m_tick
                                                                        / common::TimeSpan::GetTicksPerMSec().m_tick);
    return 0;
}

}
}
}
