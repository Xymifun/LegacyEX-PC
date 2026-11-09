#ifndef GAME_NETWORK_SERVERBOUNDUSEITEMPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDUSEITEMPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ServerboundUseItemPacket : public Packet, public boost::enable_shared_from_this<ServerboundUseItemPacket>
{
public:
    ServerboundUseItemPacket();
    ServerboundUseItemPacket(InteractionHand::EInteractionHand hand);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    InteractionHand::EInteractionHand getHand();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ServerboundUseItemPacket()); }

private:
    InteractionHand::EInteractionHand hand;
};

#endif
