#ifndef GAME_NETWORK_SERVERBOUNDCLIENTCOMMANDPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDCLIENTCOMMANDPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ServerboundClientCommandPacket : public Packet,
                                       public boost::enable_shared_from_this<ServerboundClientCommandPacket>
{
public:
    enum Action
    {
        PERFORM_RESPAWN = 0,
        REQUEST_STATS = 1,
        OPEN_INVENTORY_ACHIEVEMENT = 2
    };

    ServerboundClientCommandPacket();
    ServerboundClientCommandPacket(Action action);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    Action getAction();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ServerboundClientCommandPacket());
    }

private:
    Action action;
};

#endif
