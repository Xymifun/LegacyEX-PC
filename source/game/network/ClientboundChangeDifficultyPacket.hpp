#ifndef GAME_NETWORK_CLIENTBOUNDCHANGEDIFFICULTYPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDCHANGEDIFFICULTYPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class Difficulty;
class PacketListener;

class ClientboundChangeDifficultyPacket : public Packet,
                                          public boost::enable_shared_from_this<ClientboundChangeDifficultyPacket>
{
public:
    ClientboundChangeDifficultyPacket();
    ClientboundChangeDifficultyPacket(const Difficulty* difficulty, bool locked);
    virtual ~ClientboundChangeDifficultyPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    bool isLocked();
    const Difficulty* getDifficulty();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundChangeDifficultyPacket());
    }

private:
    const Difficulty* difficulty;
    bool locked;
};

#endif
