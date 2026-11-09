#ifndef GAME_NETWORK_SERVERBOUNDPADDLEBOATPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDPADDLEBOATPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>


#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ServerboundPaddleBoatPacket : public Packet, public boost::enable_shared_from_this<ServerboundPaddleBoatPacket>
{
public:
    ServerboundPaddleBoatPacket();
    ServerboundPaddleBoatPacket(bool left, bool right);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    bool getLeft();
    bool getRight();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ServerboundPaddleBoatPacket()); }

private:
    bool left;
    bool right;
};

#endif
