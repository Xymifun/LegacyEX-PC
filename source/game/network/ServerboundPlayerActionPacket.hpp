#ifndef GAME_NETWORK_SERVERBOUNDPLAYERACTIONPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDPLAYERACTIONPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/block/BlockPos.hpp"

class DataInputStream;
class DataOutputStream;
class Direction;
class PacketListener;

class ServerboundPlayerActionPacket : public Packet,
                                      public boost::enable_shared_from_this<ServerboundPlayerActionPacket>
{
public:
    enum Action
    {
        START_DESTROY_BLOCK,
        ABORT_DESTROY_BLOCK,
        STOP_DESTROY_BLOCK,
        DROP_ALL_ITEMS,
        DROP_ITEM,
        RELEASE_USE_ITEM,
        SWAP_ITEM_WITH_OFFHAND
    };

    ServerboundPlayerActionPacket();
    ServerboundPlayerActionPacket(Action action, const BlockPos& pos, const Direction* direction, int useTime);

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ServerboundPlayerActionPacket()); }
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual int getPacketId();

    BlockPos getPos();
    const Direction* getDirection();
    Action getAction();
    int getUseTime() const;

private:
    BlockPos mPos;
    const Direction* mDirection;
    Action mAction;
    int mUseTime;
};

#endif
