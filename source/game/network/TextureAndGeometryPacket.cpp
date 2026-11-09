#include "game/network/TextureAndGeometryPacket.hpp"

#include <sstream>

#include "game/core/DLCSkinFile.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

TextureAndGeometryPacket::TextureAndGeometryPacket()
{
    shouldDelay = true;
    textureName = L"";
    length = 0;
    data = NULL;
    additionalBoxCount = 0;
    additionalBoxes = NULL;
}

TextureAndGeometryPacket::TextureAndGeometryPacket(const std::wstring& textureName, unsigned char* data,
                                                   unsigned int length)
{
    shouldDelay = true;
    this->textureName = textureName;

    std::wstring identifier = textureName.substr(7, textureName.length());
    identifier = identifier.substr(0, identifier.find(L'.'));

    std::wstringstream stream;
    stream << std::dec << identifier.c_str();
    stream >> geometryId;
    this->data = data;
    geometryId |= 0x80000000u;
    this->length = length;
    additionalBoxCount = 0;
    additionalBoxes = NULL;
}

TextureAndGeometryPacket::TextureAndGeometryPacket(const std::wstring& textureName, unsigned char* data,
                                                   unsigned int length, DLCSkinFile* skinFile)
{
    shouldDelay = true;
    this->textureName = textureName;

    std::wstring identifier = textureName.substr(7, textureName.length());
    identifier = identifier.substr(0, identifier.find(L'.'));

    std::wstringstream stream;
    stream << std::dec << identifier.c_str();
    stream >> geometryId;
    this->data = data;
    geometryId |= 0x80000000u;
    this->length = length;

    skinAdjustments = skinFile->getSkinAdjustments();
    additionalBoxCount = skinFile->getAdditionalBoxesCount();
    if (additionalBoxCount != 0)
    {
        additionalBoxes = new SKIN_BOX[additionalBoxCount];
        std::vector<SKIN_BOX*>* boxes = skinFile->getAdditionalBoxes();
        int index = 0;
        for (std::vector<SKIN_BOX*>::iterator it = boxes->begin(); it != boxes->end(); ++it)
        {
            additionalBoxes[index] = **it;
            ++index;
        }
    }
    else
    {
        additionalBoxes = NULL;
    }
}

TextureAndGeometryPacket::TextureAndGeometryPacket(const std::wstring& textureName, unsigned char* data,
                                                   unsigned int length, std::vector<SKIN_BOX*>* additionalBoxes,
                                                   _SkinAdjustments skinAdjustments)
{
    shouldDelay = true;
    this->textureName = textureName;

    std::wstring identifier = textureName.substr(7, textureName.length());
    identifier = identifier.substr(0, identifier.find(L'.'));

    std::wstringstream stream;
    stream << std::dec << identifier.c_str();
    stream >> geometryId;
    this->data = data;
    geometryId |= 0x80000000u;
    this->length = length;
    this->skinAdjustments = skinAdjustments;

    if (additionalBoxes == NULL)
    {
        additionalBoxCount = 0;
        this->additionalBoxes = NULL;
    }
    else
    {
        additionalBoxCount = static_cast<unsigned int>(additionalBoxes->size());
        this->additionalBoxes = new SKIN_BOX[additionalBoxCount];
        int index = 0;
        for (std::vector<SKIN_BOX*>::iterator it = additionalBoxes->begin(); it != additionalBoxes->end(); ++it)
        {
            this->additionalBoxes[index] = **it;
            ++index;
        }
    }
}

TextureAndGeometryPacket::~TextureAndGeometryPacket()
{
}

int TextureAndGeometryPacket::getPacketId()
{
    return 160;
}

void TextureAndGeometryPacket::handle(PacketListener* listener)
{
    listener->handleTextureAndGeometry(shared_from_this());
}

void TextureAndGeometryPacket::read(DataInputStream* input)
{
    textureName = Packet::readUtf(input, 32767);
    geometryId = input->readInt();
    length = input->readShort();
    if (length != 0)
    {
        data = new unsigned char[length];
        for (unsigned int i = 0; i < length; i++)
        {
            data[i] = input->readByte();
        }
    }

    skinAdjustments.mXMLVersion = input->readInt();
    skinAdjustments.mAnimFlags = input->readInt();
    skinAdjustments.mGameFlags = input->readInt();
    skinAdjustments.mBodyY = input->readFloat();
    skinAdjustments.mHeadY = input->readFloat();
    skinAdjustments.mArm1Y = input->readFloat();
    skinAdjustments.mLeg1Y = input->readFloat();
    skinAdjustments.mTool1Y = input->readFloat();
    skinAdjustments.mArm0Y = input->readFloat();
    skinAdjustments.mLeg0Y = input->readFloat();
    skinAdjustments.mTool0Y = input->readFloat();

    additionalBoxCount = input->readShort();
    if (additionalBoxCount != 0)
    {
        additionalBoxes = new SKIN_BOX[additionalBoxCount];
    }

    for (unsigned int i = 0; i < additionalBoxCount; i++)
    {
        additionalBoxes[i].id = input->readShort();
        additionalBoxes[i].x = input->readFloat();
        additionalBoxes[i].y = input->readFloat();
        additionalBoxes[i].z = input->readFloat();
        additionalBoxes[i].height = input->readFloat();
        additionalBoxes[i].width = input->readFloat();
        additionalBoxes[i].depth = input->readFloat();
        additionalBoxes[i].xTexOffs = input->readFloat();
        additionalBoxes[i].yTexOffs = input->readFloat();
        additionalBoxes[i].armourFlags = input->readInt();
        additionalBoxes[i].mirror = input->readShort();
    }
}

void TextureAndGeometryPacket::write(DataOutputStream* output)
{
    Packet::writeUtf(textureName, output);
    output->writeInt(geometryId);
    output->writeShort(static_cast<short>(length));
    for (unsigned int i = 0; i < length; i++)
    {
        output->writeByte(data[i]);
    }

    output->writeInt(skinAdjustments.mXMLVersion);
    output->writeInt(skinAdjustments.mAnimFlags);
    output->writeInt(skinAdjustments.mGameFlags);
    output->writeFloat(skinAdjustments.mBodyY);
    output->writeFloat(skinAdjustments.mHeadY);
    output->writeFloat(skinAdjustments.mArm1Y);
    output->writeFloat(skinAdjustments.mLeg1Y);
    output->writeFloat(skinAdjustments.mTool1Y);
    output->writeFloat(skinAdjustments.mArm0Y);
    output->writeFloat(skinAdjustments.mLeg0Y);
    output->writeFloat(skinAdjustments.mTool0Y);
    output->writeShort(static_cast<short>(additionalBoxCount));

    for (unsigned int i = 0; i < additionalBoxCount; i++)
    {
        output->writeShort(static_cast<short>(additionalBoxes[i].id));
        output->writeFloat(additionalBoxes[i].x);
        output->writeFloat(additionalBoxes[i].y);
        output->writeFloat(additionalBoxes[i].z);
        output->writeFloat(additionalBoxes[i].height);
        output->writeFloat(additionalBoxes[i].width);
        output->writeFloat(additionalBoxes[i].depth);
        output->writeFloat(additionalBoxes[i].xTexOffs);
        output->writeFloat(additionalBoxes[i].yTexOffs);
        output->writeInt(additionalBoxes[i].armourFlags);
        output->writeShort(static_cast<short>(additionalBoxes[i].mirror));
    }
}

int TextureAndGeometryPacket::getEstimatedSize()
{
    return 4228;
}
