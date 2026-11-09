#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TIMEOUTMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TIMEOUTMANAGER_HPP

#include <functional>
#include <map>
#include <set>
#include <utility>

#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/Packet.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class PacketOut;

class TimeoutManager : public RootObject
{
public:
    TimeoutManager();
    ~TimeoutManager();

    void ServicePacketTimeouts();
    void SetQuickTimeout();
    void SetPacketQuickTimeout(PacketType type);
    void ForceSetPacketTimeouts(Time time);
    void SchedulePacketTimeout(PacketOut* pPacket);
    bool CancelPacketTimeout(PacketOut* pPacket);

private:
    typedef std::set<PacketOut*, std::less<PacketOut*>, MemAllocator<PacketOut*> > PacketSet;
    typedef std::multimap<Time, PacketOut*, std::less<Time>, MemAllocator<std::pair<const Time, PacketOut*> > >
        TimeoutMap;

    unsigned int m_unknown0;
    PacketSet m_packets;
    TimeoutMap m_timeouts;
    bool m_bQuickTimeout;
};

}
}

#endif
