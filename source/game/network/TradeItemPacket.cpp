#include "game/network/TradeItemPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

TradeItemPacket::TradeItemPacket() : containerId(0), itemIndex(0)
{
}

TradeItemPacket::TradeItemPacket(int containerIdValue, int itemIndexValue)
    : containerId(containerIdValue),
      itemIndex(itemIndexValue)
{
}

void TradeItemPacket::handle(PacketListener* listener)
{
    listener->handleTradeItem(shared_from_this());
}

void TradeItemPacket::read(DataInputStream* input)
{
    containerId = input->readInt();
    itemIndex = input->readInt();
}

void TradeItemPacket::write(DataOutputStream* output)
{
    output->writeInt(containerId);
    output->writeInt(itemIndex);
}

int TradeItemPacket::getEstimatedSize()
{
    return 8;
}
int TradeItemPacket::getPacketId()
{
    return 151;
}
