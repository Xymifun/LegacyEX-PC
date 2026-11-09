#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PACKETDISPATCHQUEUE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PACKETDISPATCHQUEUE_HPP

#include <functional>
#include <map>
#include <utility>

#include "nn/nex/LogicalClockTmpl.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class PRUDPEndPoint;
class PacketIn;

class PacketDispatchQueue : public RootObject
{
public:
    typedef LogicalClockTmpl<unsigned short, short> Clock;
    typedef std::map<Clock, PacketIn*, std::less<Clock>, MemAllocator<std::pair<const Clock, PacketIn*> > > PacketMap;

    PacketDispatchQueue();
    ~PacketDispatchQueue();

    void Queue(PacketIn* pPacket);
    PacketIn* GetNextToDispatch();
    void Dispatched(PacketIn* pPacket);
    void Purge();

    const Clock& GetNextSequence() const { return m_nextToDispatch; }

private:
    friend class PRUDPEndPoint;

    unsigned int m_unknown0;
    PacketMap m_packets;
    Clock m_nextToDispatch;
};

}
}

#endif
