#include "nn/pia/transport/ConnectionAnalysisData.hpp"
#include "nn/pia/transport/AnalysisPrinter.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

void ConnectionAnalysisData::Print(bool) const
{
    const int elapsed = static_cast<int>(m_passedMilliSecond);
    AnalysisPrinter::Write("[Analysis] ------ BEGIN(Pia Connection info, %d.%03d sec. passed) ------", elapsed / 1000,
                           elapsed % 1000);
    AnalysisPrinter::Write("[Analysis] StationIndex,  RTT,  RttMin,  RttMax,  PacketLoss");
    for (unsigned int i = 0; i < cStationNumMax; ++i)
    {
        const Entry& entry = m_entries[i];
        if (!entry.m_isValid)
            continue;
        const unsigned int sent = entry.m_sendPacketNum - entry.m_prevSendPacketNum;
        const unsigned int lost = entry.m_lostPacketNum - entry.m_prevLostPacketNum;
        const float lossRate = sent == 0 ? -1.0f : 100.0f * static_cast<float>(lost) / static_cast<float>(sent);
        AnalysisPrinter::Write("[Analysis]         0x%02x, %4d,    %4d,    %4d,      %6.2f", i,
                               static_cast<int>(entry.m_rtt), static_cast<int>(entry.m_rttMin),
                               static_cast<int>(entry.m_rttMax), static_cast<double>(lossRate));
    }
    AnalysisPrinter::Write("[Analysis] ---------------------------- END ----------------------------");
}

void ConnectionAnalysisData::Clear()
{
    for (unsigned int i = 0; i < cStationNumMax; ++i)
    {
        Entry& entry = m_entries[i];
        entry.m_rtt = 0;
        entry.m_rttMin = 0;
        entry.m_rttMax = 0;
        entry.m_prevSendPacketNum = 0;
        entry.m_sendPacketNum = 0;
        entry.m_prevLostPacketNum = 0;
        entry.m_lostPacketNum = 0;
        entry.m_isValid = false;
    }
    m_passedMilliSecond = 0;
}

}
}
}
