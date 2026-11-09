#ifndef GAME_NETWORK_SERVERBOUNDCONTAINERACKPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDCONTAINERACKPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"


class DataInputStream;
class DataOutputStream;
class PacketListener;

class ServerboundContainerAckPacket : public Packet,
                                      public boost::enable_shared_from_this<ServerboundContainerAckPacket>
{
public:
    ServerboundContainerAckPacket();
    ServerboundContainerAckPacket(int containerId, short uid, bool accepted);
    virtual ~ServerboundContainerAckPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    int getContainerId();
    int getUid();
    bool isAccepted();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ServerboundContainerAckPacket()); }

private:
    int containerId;
    short uid;
    bool accepted;
};

#endif
