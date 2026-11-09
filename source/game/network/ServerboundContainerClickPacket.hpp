#ifndef GAME_NETWORK_SERVERBOUNDCONTAINERCLICKPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDCONTAINERCLICKPACKET_HPP

#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/core/ClickType.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class ItemInstance;
class PacketListener;

class ServerboundContainerClickPacket : public Packet,
                                        public boost::enable_shared_from_this<ServerboundContainerClickPacket>
{
public:
    ServerboundContainerClickPacket();
    ServerboundContainerClickPacket(int containerId, int slotNum, int buttonNum, ClickType::eClickType clickType,
                                    not_null_ptr<ItemInstance> item, short uid);
    ServerboundContainerClickPacket(int containerId, int slotNum, int buttonNum, ClickType::eClickType clickType,
                                    std::vector<not_null_ptr<ItemInstance> > items, short uid, int numSlots);
    virtual ~ServerboundContainerClickPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    int getContainerId();
    int getSlotNum();
    int getButtonNum();
    int getUid();
    not_null_ptr<ItemInstance> getItem();
    not_null_ptr<ItemInstance> getItemAt(unsigned int index);
    std::vector<not_null_ptr<ItemInstance> > getItems();
    ClickType::eClickType getClickType();
    int getNumSlots();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ServerboundContainerClickPacket());
    }

private:
    int containerId;
    int slotNum;
    int buttonNum;
    short uid;
    std::vector<not_null_ptr<ItemInstance> > items;
    ClickType::eClickType clickType;
    int numSlots;
};

#endif
