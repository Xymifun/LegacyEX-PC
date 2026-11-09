#ifndef GAME_NETWORK_CLIENTBOUNDTAKEITEMENTITYPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDTAKEITEMENTITYPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundTakeItemEntityPacket : public Packet,
                                        public boost::enable_shared_from_this<ClientboundTakeItemEntityPacket>
{
public:
    ClientboundTakeItemEntityPacket();
    ClientboundTakeItemEntityPacket(int itemId, int playerId, int amount);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    int getItemId();
    int getPlayerId();
    int getAmount();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundTakeItemEntityPacket());
    }

private:
    int itemId;
    int playerId;
    int amount;
};

#endif
