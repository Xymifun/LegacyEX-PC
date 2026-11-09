#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PACKETHISTORY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PACKETHISTORY_HPP

#include "nn/nex/ChainPolicyHistoryPacket.hpp"
#include "nn/nex/qChain.hpp"

namespace nn
{
namespace nex
{

class ByteStream;
class Packet;
class PacketOut;
class PacketQueue;
class Time;

typedef qChain<Packet*, ChainPolicyHistoryPacket<Packet*> > PacketHistoryChain;

PacketHistoryChain::iterator PacketHistoryChainDequeue(PacketHistoryChain* pChain,
                                                       PacketHistoryChain::iterator position);
void PacketHistoryChainPurge(PacketHistoryChain* pChain);
void PacketHistoryChainQueueFront(PacketHistoryChain* pChain, PacketOut* pPacket);
void InsertToHistoryPacketQueue(PacketHistoryChain* pChain, PacketQueue* pQueue);
unsigned int CreateBunldingStreamFromHistoryPacketQueue(PacketHistoryChain* pChain, PacketOut* pPacket, Time rLimit,
                                                        unsigned int uiMaxSize, ByteStream* pStream);

}
}

#endif
