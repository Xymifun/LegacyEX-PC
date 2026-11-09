#include "game/network/ClientboundMapItemDataPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/item/MapItemSavedData.hpp"
#include "game/network/PacketListener.hpp"

ClientboundMapItemDataPacket::ClientboundMapItemDataPacket()
    : Packet(),
      boost::enable_shared_from_this<ClientboundMapItemDataPacket>()
{
}

ClientboundMapItemDataPacket::ClientboundMapItemDataPacket(int mapId, unsigned char scale, bool trackingPosition,
                                                           const std::vector<MapDecoration>& decorations,
                                                           arrayWithLength<unsigned char> colors, int startX,
                                                           int startY, int width, int height)
    : Packet(),
      boost::enable_shared_from_this<ClientboundMapItemDataPacket>()
{
    mMapId = mapId;
    mScale = scale;
    mTrackingPosition = trackingPosition;
    mDecorations = decorations;
    mStartX = startX;
    mStartY = startY;
    mWidth = width;
    mHeight = height;
    if (mWidth > 0)
    {
        mColors.length = (unsigned int)(mWidth * mHeight);
        mColors.data = new unsigned char[mColors.length]();
        for (int x = 0; x < mWidth; x++)
        {
            for (int y = 0; y < mHeight; y++)
            {
                mColors.data[y * mWidth + x] = colors.data[(mStartX + x) + (mStartY + y) * 128];
            }
        }
    }
}

ClientboundMapItemDataPacket::~ClientboundMapItemDataPacket()
{
    delete[] mColors.data;
}

void ClientboundMapItemDataPacket::handle(PacketListener* listener)
{
    listener->handleMapItemData(shared_from_this());
}

void ClientboundMapItemDataPacket::read(DataInputStream* input)
{
    mMapId = input->readVarInt();
    mScale = input->readByte();
    mTrackingPosition = input->readBoolean();

    int decorationCount = input->readVarInt();
    mDecorations = std::vector<MapDecoration>(decorationCount);
    for (int i = 0; i < decorationCount; i++)
    {
        int icon = input->readByte();
        char x = input->readByte();
        char y = input->readByte();
        int idLow = input->readByte();
        int idMid = input->readByte() << 8;
        int idHigh = input->readByte() << 16;
        unsigned char flags = input->readByte();
        char rot = input->readByte();
        mDecorations[i] = MapDecoration(MapDecoration::Type::byIcon(icon), x, y, rot,
                                        idLow | idMid | idHigh | ((flags & 7) << 24), (flags >> 7) & 1);
    }

    mWidth = input->readByte();
    if (mWidth > 0)
    {
        mHeight = input->readByte();
        mStartX = input->readByte();
        mStartY = input->readByte();
        const int length = input->readInt();
        if (length >= 0)
        {
            mColors = arrayWithLength<unsigned char>(length, true);
            input->readBytes(mColors);
        }
    }
}

void ClientboundMapItemDataPacket::write(DataOutputStream* output)
{
    output->writeVarInt(mMapId);
    output->writeByte(mScale);
    output->writeBoolean(mTrackingPosition);
    output->writeVarInt((int)mDecorations.size());
    for (std::vector<MapDecoration>::const_iterator it = mDecorations.begin(); it != mDecorations.end(); ++it)
    {
        const MapDecoration& decoration = *it;
        int id = decoration.id;
        output->writeByte(decoration.getImg());
        output->writeByte((unsigned char)decoration.getX());
        output->writeByte((unsigned char)decoration.getY());
        output->writeByte((unsigned char)id);
        output->writeByte((unsigned char)(id >> 8));
        output->writeByte((unsigned char)(id >> 16));
        output->writeByte((unsigned char)(((id >> 24) & 0x7f) | (decoration.visible ? 0x80 : 0)));
        output->writeByte((unsigned char)decoration.getRot());
    }
    output->writeByte((unsigned char)mWidth);
    if (mWidth > 0)
    {
        output->writeByte((unsigned char)mHeight);
        output->writeByte((unsigned char)mStartX);
        output->writeByte((unsigned char)mStartY);
        output->writeInt((int)mColors.length);
        output->writeBytes(mColors);
    }
}

int ClientboundMapItemDataPacket::getMapId()
{
    return mMapId;
}

void ClientboundMapItemDataPacket::applyToMap(MapItemSavedData* mapData)
{
    mapData->mScale = mScale;
    mapData->mTrackingPosition = mTrackingPosition;
    mapData->mDecorations.clear();
    for (unsigned int i = 0; i < mDecorations.size(); i++)
    {
        mapData->mDecorations[i] = mDecorations[i];
    }
    for (int x = 0; x < mWidth; x++)
    {
        for (int y = 0; y < mHeight; y++)
        {
            mapData->mColors.data[(mStartX + x) + (mStartY + y) * 128] = mColors.data[x + y * mWidth];
        }
    }
}

bool ClientboundMapItemDataPacket::isDuplicatePacket(boost::shared_ptr<ClientboundMapItemDataPacket> packet)
{
    return packet->mMapId == mMapId && packet->mStartX == mStartX && packet->mStartY == mStartY
           && packet->mWidth == mWidth && packet->mHeight == mHeight
           && packet->mDecorations.size() == mDecorations.size();
}

bool ClientboundMapItemDataPacket::tryReplaceDuplicatePacket(
    std::deque<boost::shared_ptr<Packet>, std::allocator<boost::shared_ptr<Packet> > >* queuedPackets)
{
    for (std::deque<boost::shared_ptr<Packet>, std::allocator<boost::shared_ptr<Packet> > >::iterator it =
             queuedPackets->begin();
         it != queuedPackets->end(); ++it)
    {
        if ((*it)->getPacketId() == getPacketId())
        {
            boost::shared_ptr<ClientboundMapItemDataPacket> packet =
                boost::static_pointer_cast<ClientboundMapItemDataPacket>(*it);
            if (isDuplicatePacket(packet))
            {
                it = queuedPackets->erase(it, it + 1);
                queuedPackets->insert(it, shared_from_this());
                return true;
            }
        }
    }
    return false;
}

int ClientboundMapItemDataPacket::getPacketId()
{
    return 211;
}
