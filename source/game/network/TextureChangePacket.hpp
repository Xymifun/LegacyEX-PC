#ifndef GAME_NETWORK_TEXTURECHANGEPACKET_HPP
#define GAME_NETWORK_TEXTURECHANGEPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class PacketListener;
class ServerGamePacketListenerImpl;

class TextureChangePacket : public Packet, public boost::enable_shared_from_this<TextureChangePacket>
{
    friend class ServerGamePacketListenerImpl;
    friend class ClientPacketListener;

public:
    enum ETextureChangeType
    {
        eTextureChangeType_Skin = 0,
        eTextureChangeType_Cape = 1
    };

    TextureChangePacket();
    TextureChangePacket(boost::shared_ptr<Entity> entity, ETextureChangeType type, const std::wstring& textureName);

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new TextureChangePacket()); }

    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual int getPacketId();

private:
    int mEntityId;
    ETextureChangeType mChangeType;
    std::wstring mTextureName;
};

#endif
