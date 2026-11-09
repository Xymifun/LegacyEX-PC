#ifndef GAME_NETWORK_CLIENTBOUNDCONTAINERSETSLOTPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDCONTAINERSETSLOTPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/item/ItemInstance.hpp"
#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundContainerSetSlotPacket : public Packet,
                                          public boost::enable_shared_from_this<ClientboundContainerSetSlotPacket>
{
public:
    ClientboundContainerSetSlotPacket();
    ClientboundContainerSetSlotPacket(int containerId, int slot, not_null_ptr<ItemInstance> item);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    int getContainerId();
    int getSlot();
    not_null_ptr<ItemInstance> getItem();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundContainerSetSlotPacket());
    }

private:
    int containerId;
    int slot;
    not_null_ptr<ItemInstance> item;
};

#endif
