#ifndef GAME_NETWORK_CLIENTBOUNDCONTAINERCLOSEPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDCONTAINERCLOSEPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class ClientboundContainerClosePacket : public Packet,
                                        public boost::enable_shared_from_this<ClientboundContainerClosePacket>
{
public:
    ClientboundContainerClosePacket();
    ClientboundContainerClosePacket(int containerId);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundContainerClosePacket());
    }

private:
    int containerId;
};

#endif
