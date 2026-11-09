#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ROUTINGSTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ROUTINGSTREAM_HPP

#include <stdint.h>
#include <functional>
#include <map>
#include <utility>

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/Packet.hpp"
#include "nn/nex/RoutingTable.hpp"
#include "nn/nex/Stream.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class Packet;
class PacketOut;
class RootTransport;

class RoutingStream : public Stream
{
public:
    typedef std::pair<InetAddress, InetAddress> AddressPair;
    typedef std::map<AddressPair, Time, std::less<AddressPair>, MemAllocator<std::pair<const AddressPair, Time> > >
        HistoryMap;
    typedef std::map<InetAddress, Time, std::less<InetAddress>, MemAllocator<std::pair<const InetAddress, Time> > >
        DenyMap;
    typedef bool (*RoutingFilter)(const InetAddress& rTarget, const InetAddress& rSource, Packet* pPacket);

    RoutingStream();
    virtual ~RoutingStream();

    virtual bool ReceiveIncomingPacket(unsigned short uiPort, unsigned char uiStreamID, Packet* pPacket);
    virtual void DoWork();

    static PacketOut* BuildRoutingPacket(Buffer* pPayload, PacketType type, const InetAddress& rAddress);

    void CleanHostRoutingHistory();
    unsigned int GetHistoryCount(const InetAddress* pAddress);
    void SetDenyForAddress(const InetAddress* pAddress);

    RoutingTable* GetRoutingTable() { return &m_routingTable; }

private:
    friend class Router;

    HistoryMap m_history;
    DenyMap m_denied;
    Time m_lastCleaning;
    uint32_t m_unresolved28;
    CriticalSection m_criticalSection;
    RoutingTable m_routingTable;
    RootTransport* m_pTransport;
    RoutingFilter m_pFilter;
    uint64_t m_unresolvedA0;
};

}
}

#endif
