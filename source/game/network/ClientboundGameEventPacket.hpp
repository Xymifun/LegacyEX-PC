#ifndef GAME_NETWORK_CLIENTBOUNDGAMEEVENTPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDGAMEEVENTPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundGameEventPacket : public Packet, public boost::enable_shared_from_this<ClientboundGameEventPacket>
{
public:
    enum
    {
        NO_RESPAWN_BLOCK_AVAILABLE = 0,
        START_RAINING = 1,
        STOP_RAINING = 2,
        CHANGE_GAME_MODE = 3,
        WIN_GAME = 4,
        ARROW_HIT_PLAYER = 6,
        RAIN_LEVEL_CHANGE = 7,
        THUNDER_LEVEL_CHANGE = 8,
        GUARDIAN_ELDER_EFFECT = 10,
        START_SAVING = 11,
        STOP_SAVING = 12,
        SUCCESSFUL_ARROW_HIT = 13
    };

    ClientboundGameEventPacket();
    ClientboundGameEventPacket(int event, float param);
    ClientboundGameEventPacket(int event, float param, int playerIndex);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    int getEvent();
    float getParam();
    int getPlayerIndex();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundGameEventPacket()); }

private:
    int mEvent;
    float mParam;
    int mPlayerIndex;
};

#endif
