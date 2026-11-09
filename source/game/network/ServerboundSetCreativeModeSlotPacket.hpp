#ifndef GAME_NETWORK_SERVERBOUNDSETCREATIVEMODESLOTPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDSETCREATIVEMODESLOTPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include "game/item/not_null_ptr.hpp"
#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class ItemInstance;
class PacketListener;

class ServerboundSetCreativeModeSlotPacket : public Packet,
                                             public boost::enable_shared_from_this<ServerboundSetCreativeModeSlotPacket>
{
public:
    ServerboundSetCreativeModeSlotPacket();
    ServerboundSetCreativeModeSlotPacket(int slot, not_null_ptr<ItemInstance> item);
    virtual void handle(PacketListener* listener);
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual int getEstimatedSize();
    int getSlotNum();
    not_null_ptr<ItemInstance> getItem();
    virtual int getPacketId();
    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ServerboundSetCreativeModeSlotPacket());
    }

private:
    int slot;
    not_null_ptr<ItemInstance> item;
};

#endif
