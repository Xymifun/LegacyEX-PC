#ifndef GAME_NETWORK_VOTEPACKET_HPP
#define GAME_NETWORK_VOTEPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class VotePacket : public Packet, public boost::enable_shared_from_this<VotePacket>
{
public:
    VotePacket();
    VotePacket(int voteableId, char vote, int playerId, int voteId);
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new VotePacket()); }
    int voteableId;
    char vote;
    int playerId;
    int voteId;
};

#endif
