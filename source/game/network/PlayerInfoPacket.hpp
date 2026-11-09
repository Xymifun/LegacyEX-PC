#ifndef GAME_NETWORK_PLAYERINFOPACKET_HPP
#define GAME_NETWORK_PLAYERINFOPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;
class ServerGamePacketListenerImpl;
class ServerPlayer;

class PlayerInfoPacket : public Packet, public boost::enable_shared_from_this<PlayerInfoPacket>
{
public:
    PlayerInfoPacket();
    PlayerInfoPacket(unsigned char playerId, short playerIndex, unsigned int privileges, unsigned char gameType);
    PlayerInfoPacket(boost::shared_ptr<ServerPlayer> player);
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new PlayerInfoPacket()); }

private:
    friend class ClientPacketListener;
    friend class ServerGamePacketListenerImpl;

    short mPlayerId;
    short mPlayerIndex;
    unsigned int mPrivileges;
    int mEntityId;
    unsigned char mGameType;
};

#endif
