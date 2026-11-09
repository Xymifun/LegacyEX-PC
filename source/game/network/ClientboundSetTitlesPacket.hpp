#ifndef GAME_NETWORK_CLIENTBOUNDSETTITLESPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDSETTITLESPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundSetTitlesPacket : public Packet, public boost::enable_shared_from_this<ClientboundSetTitlesPacket>
{
public:
    enum Type
    {
        TITLE,
        SUBTITLE,
        ACTIONBAR,
        TIMES,
        CLEAR,
        RESET
    };

    ClientboundSetTitlesPacket();
    ClientboundSetTitlesPacket(Type type, const std::wstring& text, int fadeIn, int stay, int fadeOut);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundSetTitlesPacket()); }

private:
    unsigned int type;
    std::wstring text;
    int fadeIn;
    int stay;
    int fadeOut;
};

#endif
