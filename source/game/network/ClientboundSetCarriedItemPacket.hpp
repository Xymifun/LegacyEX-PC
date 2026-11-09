#ifndef GAME_NETWORK_CLIENTBOUNDSETCARRIEDITEMPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDSETCARRIEDITEMPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundSetCarriedItemPacket : public Packet,
                                        public boost::enable_shared_from_this<ClientboundSetCarriedItemPacket>
{
public:
    ClientboundSetCarriedItemPacket();
    ClientboundSetCarriedItemPacket(int slot);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(boost::shared_ptr<Packet> packet);

    int getSlot();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundSetCarriedItemPacket());
    }

private:
    int slot;
};

#endif
