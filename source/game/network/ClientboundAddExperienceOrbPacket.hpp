#ifndef GAME_NETWORK_CLIENTBOUNDADDEXPERIENCEORBPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDADDEXPERIENCEORBPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class ExperienceOrb;
class PacketListener;

class ClientboundAddExperienceOrbPacket : public Packet,
                                          public boost::enable_shared_from_this<ClientboundAddExperienceOrbPacket>
{
public:
    ClientboundAddExperienceOrbPacket();
    ClientboundAddExperienceOrbPacket(boost::shared_ptr<ExperienceOrb> entity, int, int, int x, int y, int z);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    int getId();
    int getX();
    int getY();
    int getZ();
    int getValue();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundAddExperienceOrbPacket());
    }

private:
    int mId;
    int mX;
    int mY;
    int mZ;
    int mValue;
};

#endif
