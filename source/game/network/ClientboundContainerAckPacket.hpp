#ifndef GAME_NETWORK_CLIENTBOUNDCONTAINERACKPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDCONTAINERACKPACKET_HPP

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundContainerAckPacket : public Packet,
                                      public boost::enable_shared_from_this<ClientboundContainerAckPacket>
{
public:
    ClientboundContainerAckPacket();
    ClientboundContainerAckPacket(int containerId, short uid, bool accepted);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    int getContainerId();
    short getUid();
    bool isAccepted();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundContainerAckPacket()); }

private:
    int containerId;
    short uid;
    bool accepted;
};

#endif
