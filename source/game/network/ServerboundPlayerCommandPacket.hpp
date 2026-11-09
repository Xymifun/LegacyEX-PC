#ifndef GAME_NETWORK_SERVERBOUNDPLAYERCOMMANDPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDPLAYERCOMMANDPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class PacketListener;

class ServerboundPlayerCommandPacket : public Packet,
                                       public boost::enable_shared_from_this<ServerboundPlayerCommandPacket>
{
public:
    enum Action
    {
        PRESS_SHIFT_KEY = 0,
        RELEASE_SHIFT_KEY = 1,
        STOP_SLEEPING = 2,
        START_SPRINTING = 3,
        STOP_SPRINTING = 4,
        RIDING_JUMP = 5,
        STOP_RIDING_JUMP = 6,
        OPEN_INVENTORY = 7,
        START_FALL_FLYING = 8,
        START_SWIMMING = 9,
        STOP_SWIMMING = 10,
        START_SPIN_ATTACK = 11,
        STOP_SPIN_ATTACK = 12
    };
    ServerboundPlayerCommandPacket();
    ServerboundPlayerCommandPacket(boost::shared_ptr<Entity> entity, Action action);
    ServerboundPlayerCommandPacket(boost::shared_ptr<Entity> entity, Action action, int data);
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    Action getAction();
    int getData();
    int getId();
    virtual int getPacketId();
    static boost::shared_ptr<Packet> create();

private:
    int id;
    Action action;
    int data;
};

#endif
