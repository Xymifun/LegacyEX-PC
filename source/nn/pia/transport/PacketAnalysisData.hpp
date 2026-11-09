#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PACKET_ANALYSIS_DATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PACKET_ANALYSIS_DATA_HPP

#include "nn/pia/Reconstruction.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

class PacketAnalysisData
{
public:
    enum
    {
        cEntryNumMax = 32,
        cNameLengthMax = 32
    };

    class Entry
    {
    public:
        Entry();

        ProtocolId m_protocolId;
        unsigned int m_packetNum;
        unsigned int m_packetSize;
        unsigned int m_totalPacketNum;
        unsigned long long m_totalPacketSize;
    };

    void ClearExceptName();
    void ClearCounters();
    void Print(bool isDetail) const;
    int GetIndex(ProtocolId protocolId) const;
    int CreateIndex(ProtocolId protocolId);
    void Trace(unsigned long long value) const;

    Entry m_entries[cEntryNumMax];
    int m_entryNum;
    unsigned int m_passedMilliSecond;
    char m_name[cNameLengthMax];
    unsigned int m_packetNum;
    unsigned int m_packetSize;
    unsigned int m_totalPacketNum;
    unsigned long long m_totalPacketSize;
};

}
}
}

#endif
