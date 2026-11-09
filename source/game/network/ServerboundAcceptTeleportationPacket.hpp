#ifndef GAME_NETWORK_SERVERBOUNDACCEPTTELEPORTATIONPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDACCEPTTELEPORTATIONPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ServerboundAcceptTeleportationPacket : public Packet,
                                             public boost::enable_shared_from_this<ServerboundAcceptTeleportationPacket>
{
public:
    ServerboundAcceptTeleportationPacket();
    ServerboundAcceptTeleportationPacket(int id);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    int getId();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ServerboundAcceptTeleportationPacket());
    }

private:
    int id;
};

#endif
