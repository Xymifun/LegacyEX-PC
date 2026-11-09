#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PACKETQUEUE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PACKETQUEUE_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/qChain.hpp"

namespace nn
{
namespace nex
{

class CriticalSection;
class Packet;

class PacketQueue : public RootObject
{
public:
    typedef qChain<Packet*, DefaultChainPolicy<Packet*> > Chain;

    explicit PacketQueue(const wchar_t* pName);
    virtual ~PacketQueue();

    virtual void Purge();
    virtual void Queue(Packet* pPacket, bool bStampTime);
    virtual void QueueFront(Packet* pPacket, bool bStampTime);
    virtual Chain::iterator Dequeue(Chain::iterator position);

    virtual CriticalSection* GetLock() { return 0; }

    Chain::iterator begin() const { return m_packets.begin(); }

    Chain::iterator end() const { return m_packets.end(); }

    bool empty() const { return m_packets.empty(); }

    unsigned int size() const { return m_packets.size(); }

    Packet* front() { return m_packets.front(); }

private:
    Chain m_packets;
};

}
}

#endif
