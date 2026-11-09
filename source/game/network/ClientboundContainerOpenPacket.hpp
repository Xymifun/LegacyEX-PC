#ifndef GAME_NETWORK_CLIENTBOUNDCONTAINEROPENPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDCONTAINEROPENPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <stdint.h>
#include <string>

#include "game/core/InteractionObject.hpp"
#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundContainerOpenPacket : public Packet,
                                       public boost::enable_shared_from_this<ClientboundContainerOpenPacket>
{
public:
    ClientboundContainerOpenPacket();
    ClientboundContainerOpenPacket(int containerId, int type, InteractionObject::NameData data);
    ClientboundContainerOpenPacket(int containerId, int type, InteractionObject::NameData data, int size);
    ClientboundContainerOpenPacket(int containerId, int type, InteractionObject::NameData data, int size, int entityId);
    virtual ~ClientboundContainerOpenPacket();

    void _init(int containerId, int type, InteractionObject::NameData data, int size, int entityId);

    virtual void handle(PacketListener* listener);
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual int getEstimatedSize();

    int getContainerId();
    int getType();
    bool hasCustomName();
    std::wstring getTitle();
    int getSize();
    int getEntityId();
    uint32_t isContainer();
    virtual int getPacketId();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundContainerOpenPacket());
    }

    static const int CONTAINER = 0;
    static const int WORKBENCH = 1;
    static const int FURNACE = 2;
    static const int ENCHANTMENT = 4;
    static const int BREWING_STAND = 5;
    static const int TRADER_NPC = 6;
    static const int BEACON = 7;
    static const int HOPPER = 9;
    static const int DROPPER = 10;
    static const int HORSE = 11;
    static const int FIREWORKS = 12;
    static const int BONUS_CHEST = 13;
    static const int LARGE_CHEST = 14;
    static const int ENDER_CHEST = 15;
    static const int MINECART_CHEST = 16;
    static const int MINECART_HOPPER = 17;
    static const int REPAIR_TABLE = 19;
    static const int TRAP = 20;
    static const int SHULKER_BOX = 21;

private:
    int mContainerId;
    int mType;
    int mSize;
    bool mHasCustomName;
    int mEntityId;
    InteractionObject::NameData mNameData;
};

#endif
