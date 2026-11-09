#ifndef GAME_NETWORK_CLIENTBOUNDSETEXPERIENCEPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDSETEXPERIENCEPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundSetExperiencePacket : public Packet,
                                       public boost::enable_shared_from_this<ClientboundSetExperiencePacket>
{
public:
    ClientboundSetExperiencePacket();
    ClientboundSetExperiencePacket(float experienceProgress, int totalExperience, int experienceLevel);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(boost::shared_ptr<Packet> packet);

    float getExperienceProgress();
    int getTotalExperience();
    int getExperienceLevel();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundSetExperiencePacket());
    }

private:
    float experienceProgress;
    int totalExperience;
    int experienceLevel;
};

#endif
