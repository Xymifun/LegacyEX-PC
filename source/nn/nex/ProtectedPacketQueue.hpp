#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PROTECTEDPACKETQUEUE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PROTECTEDPACKETQUEUE_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/PacketQueue.hpp"

namespace nn
{
namespace nex
{

class ProtectedPacketQueue : public PacketQueue
{
public:
    ProtectedPacketQueue(const wchar_t* pName, CriticalSection* pLock) : PacketQueue(pName), m_pLock(pLock) {}

    virtual ~ProtectedPacketQueue();

    virtual void Purge();
    virtual void Queue(Packet* pPacket, bool bStampTime);
    virtual void QueueFront(Packet* pPacket, bool bStampTime);
    virtual Chain::iterator Dequeue(Chain::iterator position);
    virtual CriticalSection* GetLock();

private:
    CriticalSection* m_pLock;
};

}
}

#endif
