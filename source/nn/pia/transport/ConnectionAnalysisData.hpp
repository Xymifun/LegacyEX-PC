#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_CONNECTION_ANALYSIS_DATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_CONNECTION_ANALYSIS_DATA_HPP

namespace nn
{
namespace pia
{
namespace transport
{

class ConnectionAnalysisData
{
public:
    enum
    {
        cStationNumMax = 32
    };

    class Entry
    {
    public:
        unsigned int m_rtt;
        unsigned int m_rttMin;
        unsigned int m_rttMax;
        unsigned int m_prevSendPacketNum;
        unsigned int m_sendPacketNum;
        unsigned int m_prevLostPacketNum;
        unsigned int m_lostPacketNum;
        bool m_isValid;
    };

    void Clear();
    void Print(bool isDetail) const;
    void Trace(unsigned long long value) const;

    Entry m_entries[cStationNumMax];
    unsigned int m_passedMilliSecond;
};

}
}
}

#endif
