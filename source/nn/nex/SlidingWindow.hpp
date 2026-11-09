#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SLIDINGWINDOW_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SLIDINGWINDOW_HPP

#include <functional>
#include <map>
#include <utility>

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/LogicalClockTmpl.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/qMap.hpp"

namespace nn
{
namespace nex
{

class PacketEncDec;
class PacketOut;
class Stream;

class SlidingWindow : public RootObject
{
public:
    typedef LogicalClockTmpl<unsigned short, short> Clock;
    typedef std::map<Clock, PacketOut*, std::less<Clock>, MemAllocator<std::pair<const Clock, PacketOut*> > > PacketMap;

    SlidingWindow(unsigned short uiWindowSize, unsigned int uiMaxBufferedPackets);
    ~SlidingWindow();

    bool Push(PacketOut* pPacket, PacketEncDec* pEncDec, Stream* pStream);
    PacketOut* GetNextToSend();
    PacketOut* GetPacket(Clock clock);
    PacketOut* GetPacket(PacketMap::iterator position);
    void Acknowledged(Clock clock, qMap<unsigned int, unsigned int>& rPendingCounts);
    void Purge();
    bool Empty();
    bool ReadyToSend();
    bool DataPending();
    unsigned int NbDataPending();
    int GetFreeBufferNum() const;
    PacketMap::iterator AcquireIterator();
    void ReleaseIterator();
    void Trace(unsigned long long uiFilter);

    unsigned int GetBufferedPacketCount() const { return m_packets.size(); }

    const Clock& GetOldestUnacknowledged() const { return m_oldestUnacknowledged; }

private:
    unsigned int m_unknown0;
    PacketMap m_packets;
    unsigned short m_uiWindowSize;
    Clock m_nextSequence;
    Clock m_nextToSend;
    Clock m_oldestUnacknowledged;
    unsigned int m_uiDataPending;
    unsigned int m_uiMaxBufferedPackets;
    CriticalSection m_criticalSection;
};

}
}

#endif
