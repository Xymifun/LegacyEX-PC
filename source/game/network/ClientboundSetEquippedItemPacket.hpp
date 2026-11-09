#ifndef GAME_NETWORK_CLIENTBOUNDSETEQUIPPEDITEMPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDSETEQUIPPEDITEMPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include "game/network/Packet.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/item/not_null_ptr.hpp"

class DataInputStream;
class DataOutputStream;
class ItemInstance;
class PacketListener;

class ClientboundSetEquippedItemPacket : public Packet,
                                         public boost::enable_shared_from_this<ClientboundSetEquippedItemPacket>
{
public:
    ClientboundSetEquippedItemPacket();
    ClientboundSetEquippedItemPacket(int entity, const EquipmentSlot* slot, not_null_ptr<ItemInstance> item);
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(boost::shared_ptr<Packet> packet);
    virtual int getPacketId();
    not_null_ptr<ItemInstance> getItem();
    int getEntity();
    const EquipmentSlot* getSlot();
    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundSetEquippedItemPacket());
    }

private:
    not_null_ptr<ItemInstance> m_item;
    int m_entity;
    const EquipmentSlot* m_slot;
};

#endif
