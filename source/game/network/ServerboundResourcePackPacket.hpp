#ifndef GAME_NETWORK_SERVERBOUNDRESOURCEPACKPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDRESOURCEPACKPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ServerboundResourcePackPacket : public Packet,
                                      public boost::enable_shared_from_this<ServerboundResourcePackPacket>
{
public:
    enum Action
    {
        SUCCESSFULLY_LOADED = 0,
        DECLINED = 1,
        FAILED_DOWNLOAD = 2,
        ACCEPTED = 3
    };

    ServerboundResourcePackPacket();
    ServerboundResourcePackPacket(Action action);
    virtual ~ServerboundResourcePackPacket();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ServerboundResourcePackPacket()); }

    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getPacketId();

    Action getAction();

private:
    Action mAction;
};

#endif
