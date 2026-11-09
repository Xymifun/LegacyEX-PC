#ifndef GAME_NETWORK_SERVERBOUNDCONTAINERCLOSEPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDCONTAINERCLOSEPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ServerboundContainerClosePacket : public Packet,
                                        public boost::enable_shared_from_this<ServerboundContainerClosePacket>
{
public:
    ServerboundContainerClosePacket();
    ServerboundContainerClosePacket(int containerId);
    virtual ~ServerboundContainerClosePacket();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ServerboundContainerClosePacket());
    }

    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getPacketId();

    int getContainerId();

private:
    int mContainerId;
};

#endif
