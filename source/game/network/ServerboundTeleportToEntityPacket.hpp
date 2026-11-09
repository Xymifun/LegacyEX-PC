#ifndef GAME_NETWORK_SERVERBOUNDTELEPORTTOENTITYPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDTELEPORTTOENTITYPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/generation/PlayerUID.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class PacketListener;
class ServerLevel;
class ServerPlayer;

class ServerboundTeleportToEntityPacket : public Packet,
                                          public boost::enable_shared_from_this<ServerboundTeleportToEntityPacket>
{
public:
    ServerboundTeleportToEntityPacket();
    ServerboundTeleportToEntityPacket(PlayerUID playerUID);
    virtual ~ServerboundTeleportToEntityPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    boost::shared_ptr<Entity> getEntity(ServerLevel* level);

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ServerboundTeleportToEntityPacket());
    }

private:
    PlayerUID mPlayerUID;
};

#endif
