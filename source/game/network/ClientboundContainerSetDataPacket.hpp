#ifndef GAME_NETWORK_CLIENTBOUNDCONTAINERSETDATAPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDCONTAINERSETDATAPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundContainerSetDataPacket : public Packet,
                                          public boost::enable_shared_from_this<ClientboundContainerSetDataPacket>
{
public:
    ClientboundContainerSetDataPacket();
    ClientboundContainerSetDataPacket(int containerId, int id, int value);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    int getContainerId();
    int getId();
    int getValue();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundContainerSetDataPacket());
    }

private:
    int containerId;
    int id;
    int value;
};

#endif
