#ifndef GAME_NETWORK_SERVERBOUNDINTERACTPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDINTERACTPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class Level;
class PacketListener;
class Vec3;

class ServerboundInteractPacket : public Packet, public boost::enable_shared_from_this<ServerboundInteractPacket>
{
public:
    enum Action
    {
        INTERACT,
        ATTACK,
        INTERACT_AT
    };

    ServerboundInteractPacket();
    ServerboundInteractPacket(boost::shared_ptr<Entity> target);
    ServerboundInteractPacket(boost::shared_ptr<Entity> target, InteractionHand::EInteractionHand hand);
    ServerboundInteractPacket(boost::shared_ptr<Entity> target, InteractionHand::EInteractionHand hand, Vec3* location);
    virtual ~ServerboundInteractPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    boost::shared_ptr<Entity> getTarget(Level* level);
    Action getAction();
    InteractionHand::EInteractionHand getHand();
    Vec3* getLocation();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ServerboundInteractPacket()); }

private:
    int entityId;
    Action action;
    Vec3* location;
    InteractionHand::EInteractionHand hand;
};

#endif
