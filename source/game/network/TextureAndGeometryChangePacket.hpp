#ifndef GAME_NETWORK_TEXTUREANDGEOMETRYCHANGEPACKET_HPP
#define GAME_NETWORK_TEXTUREANDGEOMETRYCHANGEPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class PacketListener;

class TextureAndGeometryChangePacket : public Packet,
                                       public boost::enable_shared_from_this<TextureAndGeometryChangePacket>
{
public:
    TextureAndGeometryChangePacket();
    TextureAndGeometryChangePacket(boost::shared_ptr<Entity> entity, const std::wstring& geometryName);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new TextureAndGeometryChangePacket());
    }

public:
    int entityId;
    std::wstring geometryName;
    unsigned int geometryId;
};

#endif
