#ifndef GAME_NETWORK_SERVERBOUNDCONTAINERBUTTONCLICKPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDCONTAINERBUTTONCLICKPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ServerboundContainerButtonClickPacket
    : public Packet,
      public boost::enable_shared_from_this<ServerboundContainerButtonClickPacket>
{
public:
    ServerboundContainerButtonClickPacket();
    ServerboundContainerButtonClickPacket(int containerId, int buttonId);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    int getContainerId();
    int getButtonId();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ServerboundContainerButtonClickPacket());
    }

private:
    int containerId;
    int buttonId;
};

#endif
