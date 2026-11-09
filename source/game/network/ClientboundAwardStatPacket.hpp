#ifndef GAME_NETWORK_CLIENTBOUNDAWARDSTATPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDAWARDSTATPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundAwardStatPacket : public Packet, public boost::enable_shared_from_this<ClientboundAwardStatPacket>
{
public:
    ClientboundAwardStatPacket();
    ClientboundAwardStatPacket(int statId, int count);
    ClientboundAwardStatPacket(int statId, arrayWithLength<unsigned char> paramData);
    virtual ~ClientboundAwardStatPacket();
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool isAync();
    int getCount();
    arrayWithLength<unsigned char> getParamData();
    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundAwardStatPacket()); }

private:
    friend class ClientPacketListener;
    int statId;
    arrayWithLength<unsigned char> paramData;
};

#endif
