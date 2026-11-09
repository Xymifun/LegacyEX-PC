#ifndef GAME_NETWORK_TEXTUREPACKET_HPP
#define GAME_NETWORK_TEXTUREPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class TexturePacket : public Packet, public boost::enable_shared_from_this<TexturePacket>
{
public:
    TexturePacket();
    TexturePacket(const std::wstring& textureName, unsigned char* data, unsigned int length);
    virtual ~TexturePacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new TexturePacket()); }

public:
    std::wstring textureName;
    unsigned char* data;
    unsigned int length;
};

#endif
