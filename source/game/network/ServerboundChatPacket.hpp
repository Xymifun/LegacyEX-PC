#ifndef GAME_NETWORK_SERVERBOUNDCHATPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDCHATPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ServerboundChatPacket : public Packet, public boost::enable_shared_from_this<ServerboundChatPacket>
{
public:
    ServerboundChatPacket();
    ServerboundChatPacket(const std::wstring& message);
    virtual ~ServerboundChatPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    std::wstring getMessage();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ServerboundChatPacket()); }

private:
    std::wstring message;
};

#endif
