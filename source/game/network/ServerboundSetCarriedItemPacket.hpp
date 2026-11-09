#ifndef GAME_NETWORK_SERVERBOUNDSETCARRIEDITEMPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDSETCARRIEDITEMPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ServerboundSetCarriedItemPacket : public Packet,
                                        public boost::enable_shared_from_this<ServerboundSetCarriedItemPacket>
{
public:
    ServerboundSetCarriedItemPacket();
    ServerboundSetCarriedItemPacket(int slot);
    virtual ~ServerboundSetCarriedItemPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    int getSlot();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ServerboundSetCarriedItemPacket());
    }

private:
    int slot;
};

#endif
