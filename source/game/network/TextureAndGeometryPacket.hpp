#ifndef GAME_NETWORK_TEXTUREANDGEOMETRYPACKET_HPP
#define GAME_NETWORK_TEXTUREANDGEOMETRYPACKET_HPP

#include <string>
#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "vendor/SKIN_BOX.hpp"
#include "vendor/_SkinAdjustments.hpp"

class DataInputStream;
class DataOutputStream;
class DLCSkinFile;
class PacketListener;

class TextureAndGeometryPacket : public Packet, public boost::enable_shared_from_this<TextureAndGeometryPacket>
{
public:
    TextureAndGeometryPacket();
    TextureAndGeometryPacket(const std::wstring& textureName, unsigned char* data, unsigned int length);
    TextureAndGeometryPacket(const std::wstring& textureName, unsigned char* data, unsigned int length,
                             DLCSkinFile* skinFile);
    TextureAndGeometryPacket(const std::wstring& textureName, unsigned char* data, unsigned int length,
                             std::vector<SKIN_BOX*>* additionalBoxes, _SkinAdjustments skinAdjustments);
    virtual ~TextureAndGeometryPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new TextureAndGeometryPacket()); }

public:
    std::wstring textureName;
    unsigned int geometryId;
    unsigned char* data;
    unsigned int length;
    SKIN_BOX* additionalBoxes;
    unsigned int additionalBoxCount;
    _SkinAdjustments skinAdjustments;
};

#endif
