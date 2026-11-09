#ifndef GAME_NETWORK_GAMECOMMANDPACKET_HPP
#define GAME_NETWORK_GAMECOMMANDPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/command/EGameCommand.hpp"
#include "game/network/Packet.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class GameCommandPacket : public Packet, public boost::enable_shared_from_this<GameCommandPacket>
{
public:
    GameCommandPacket();
    GameCommandPacket(EGameCommand command, arrayWithLength<unsigned char> data);
    virtual ~GameCommandPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new GameCommandPacket()); }

    EGameCommand command;
    int dataLength;
    arrayWithLength<unsigned char> data;
};

#endif
