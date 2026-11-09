#ifndef GAME_NETWORK_SERVERBOUNDUSEITEMONPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDUSEITEMONPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/network/Packet.hpp"
#include "game/world/block/BlockPos.hpp"

class DataInputStream;
class DataOutputStream;
class Direction;
class PacketListener;

class ServerboundUseItemOnPacket : public Packet, public boost::enable_shared_from_this<ServerboundUseItemOnPacket>
{
public:
    ServerboundUseItemOnPacket();
    ServerboundUseItemOnPacket(const BlockPos& pos, const Direction* face, InteractionHand::EInteractionHand hand,
                               float clickX, float clickY, float clickZ);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    BlockPos getPos();
    const Direction* getFace();
    InteractionHand::EInteractionHand getHand();
    double getClickX();
    double getClickY();
    double getClickZ();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ServerboundUseItemOnPacket()); }

private:
    BlockPos pos;
    const Direction* face;
    InteractionHand::EInteractionHand hand;
    float clickX;
    float clickY;
    float clickZ;
};

#endif
