#ifndef GAME_NETWORK_CLIENTBOUNDRESOURCEPACKPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDRESOURCEPACKPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundResourcePackPacket : public Packet,
                                      public boost::enable_shared_from_this<ClientboundResourcePackPacket>
{
public:
    ClientboundResourcePackPacket();
    ClientboundResourcePackPacket(const std::wstring& url, const std::wstring& hash);
    virtual ~ClientboundResourcePackPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    std::wstring getUrl();
    std::wstring getHash();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundResourcePackPacket()); }

private:
    std::wstring url;
    std::wstring hash;
};

#endif
