#include "game/item/MapItemSavedData.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include "game/core/Direction.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/HangingEntity.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Math.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"

static const int STRONGHOLD_DECORATION_ID = -1;

MapItemSavedData::MapItemSavedData(const std::wstring& id) : SavedData(id)
{
    mZCenter = 0;
    mScale = 0;
    mXCenter = 0;
    mDimension = 0;
    mTrackingPosition = true;
    mColors = arrayWithLength<unsigned char>(16384, true);
    mLastUpdateTime = 0;
}

MapItemSavedData::~MapItemSavedData()
{
    delete mColors.data;
}

void MapItemSavedData::setOrigin(double x, double z, int scale)
{
    int size = (1 << scale) << 7;
    mXCenter = (int)Math::round(x / (double)size) * size;
    mZCenter = (int)Math::round(z / (double)size) * size;
}

boost::shared_ptr<MapItemSavedData::HoldingPlayer> MapItemSavedData::getHoldingPlayer(boost::shared_ptr<Player> player)
{
    boost::shared_ptr<HoldingPlayer> holding;
    boost::unordered_map<boost::shared_ptr<Player>, boost::shared_ptr<HoldingPlayer>, _PlayerKeyHash,
                         _PlayerKeyEq>::iterator it = mCarriedBy.find(player);
    if (it == mCarriedBy.end())
    {
        holding = boost::shared_ptr<HoldingPlayer>(new HoldingPlayer(player, this));
        mCarriedBy[player] = holding;
        mHoldingPlayers.push_back(holding);
    }
    else
    {
        holding = it->second;
    }
    return holding;
}

void MapItemSavedData::setDirty(int x, int y)
{
    SavedData::setDirty();
    std::vector<boost::shared_ptr<HoldingPlayer> >::iterator end = mHoldingPlayers.end();
    for (std::vector<boost::shared_ptr<HoldingPlayer> >::iterator it = mHoldingPlayers.begin(); it != end; ++it)
    {
        boost::shared_ptr<HoldingPlayer> holding = *it;
        holding->markDirty(x, y);
    }
}

int MapItemSavedData::fjEnumerateAxisPoint(int point)
{
    int value = point / (1 << (mScale & 0x3f)) + 0x40;
    return value - (((value >> 31) & 0x7f) + value & ~0x7f);
}

int MapItemSavedData::fjGetDecorationId(CompoundTag* tag)
{
    int type = tag->getByte(L"type");
    int x = fjEnumerateAxisPoint((int)tag->getDouble(L"x") - mXCenter);
    int z = fjEnumerateAxisPoint((int)tag->getDouble(L"z") - mZCenter);
    return -(x * 0x80 + z + type);
}

void MapItemSavedData::addDecoration(const MapDecoration::Type* type, Level* level, int id, double x, double, double z,
                                     double rotation, bool visible)
{
    float scale = (float)(1 << (mScale & 0x3f));
    float mapX = (float)(x - (double)mXCenter) / scale;
    float mapZ = (float)(z - (double)mZCenter) / scale;
    char imageX = (char)((double)(mapX * 2.0f) + 0.5);
    char imageZ = (char)((double)(mapZ * 2.0f) + 0.5);
    char rot;
    bool isPlayerType = type->isPlayer();
    boost::unordered_map<int, MapDecoration>* decorations = isPlayerType ? &mDecorations : &mDecorationsById;

    if (mapX >= -63.0f && mapZ >= -63.0f && mapX <= 63.0f && mapZ <= 63.0f)
    {
        rotation += rotation < 0.0 ? -8.0 : 8.0;
        rot = (char)(rotation * 16.0 / 360.0);
        if (mDimension < 0)
        {
            int time = (int)(level->getLevelData()->getDayTime() / 10);
            rot = (char)((unsigned int)(time * time * 34187121 + time * 121) >> 15 & 15);
        }
    }
    else
    {
        if (isPlayerType)
        {
            if (std::abs(mapX) < 320.0f && std::abs(mapZ) < 320.0f)
            {
                type = &MapDecoration::Type::TYPES[16 + type->getIcon()];
            }
            else if (!mUnlimitedTracking)
            {
                decorations->erase(id);
                return;
            }
        }
        else if (type != &MapDecoration::Type::TYPES[4])
        {
            decorations->erase(id);
            return;
        }

        rot = 0;
        if (mapX <= -63.0f)
            imageX = -128;
        if (mapZ <= -63.0f)
            imageZ = -128;
        if (mapX >= 63.0f)
            imageX = 127;
        if (mapZ >= 63.0f)
            imageZ = 127;
    }

    MemSect(0x2d);
    (*decorations)[id] = MapDecoration(type, imageX, imageZ, rot, id, visible);
    MemSect(0);
}

void MapItemSavedData::tickCarriedBy(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item)
{
    PIXBeginNamedEvent(0.0f, "MapItemSavedData::tickCarriedBy");

    if (mCarriedBy.find(player) == mCarriedBy.end())
    {
        boost::shared_ptr<HoldingPlayer> holding(new HoldingPlayer(player, this));
        mCarriedBy[player] = holding;
        mHoldingPlayers.push_back(holding);
    }

    mDecorations.clear();

    std::vector<boost::shared_ptr<HoldingPlayer> >::iterator it = mHoldingPlayers.begin();
    while (it != mHoldingPlayers.end())
    {
        boost::shared_ptr<HoldingPlayer> holding = *it;
        if (holding->mPlayer->mRemoved)
        {
            boost::unordered_map<boost::shared_ptr<Player>, boost::shared_ptr<HoldingPlayer>, _PlayerKeyHash,
                                 _PlayerKeyEq>::iterator carried = mCarriedBy.find(holding->mPlayer);
            if (carried != mCarriedBy.end())
                mCarriedBy.erase(carried);
            it = mHoldingPlayers.erase(it);
        }
        else
        {
            Level* level = holding->mPlayer->mLevel;
            if (!level->mIsClientSide && holding->mPlayer->mDimension == 0
                && (level->getLevelData()->getHasStrongholdEndPortal() || level->getLevelData()->getHasStronghold()))
            {
                bool playerInEnd = false;
                PlayerList* playerList = gMinecraftServer != NULL ? gMinecraftServer->mPlayers : NULL;
                for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator other = playerList->mPlayers.begin();
                     other != playerList->mPlayers.end(); ++other)
                {
                    boost::shared_ptr<ServerPlayer> serverPlayer = *other;
                    if (serverPlayer->mDimension == 1)
                    {
                        playerInEnd = true;
                        break;
                    }
                }

                boost::unordered_map<int, MapDecoration>::iterator marker = mDecorationsById.find(
                    STRONGHOLD_DECORATION_ID);
                if (marker == mDecorationsById.end())
                {
                    if (playerInEnd)
                    {
                        int markerX;
                        int markerZ;
                        if (level->getLevelData()->getHasStrongholdEndPortal())
                        {
                            markerX = level->getLevelData()->getXStrongholdEndPortal();
                            markerZ = level->getLevelData()->getZStrongholdEndPortal();
                        }
                        else
                        {
                            markerX = level->getLevelData()->getXStronghold() << 4;
                            markerZ = level->getLevelData()->getZStronghold() << 4;
                        }
                        addDecoration(&MapDecoration::Type::TYPES[4], holding->mPlayer->mLevel,
                                      STRONGHOLD_DECORATION_ID, (double)markerX, 0.0, (double)markerZ, 0.0, true);
                    }
                }
                else if (!playerInEnd)
                {
                    mDecorationsById.erase(marker);
                }
            }
            ++it;
        }
    }

    PlayerList* playerList = gMinecraftServer != NULL ? gMinecraftServer->mPlayers : NULL;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator other = playerList->mPlayers.begin();
         other != playerList->mPlayers.end(); ++other)
    {
        boost::shared_ptr<ServerPlayer> target = *other;
        if (target != NULL && target->mDimension == mDimension && mTrackingPosition)
        {
            bool visible = true;
            if (target != player)
            {
                bool shown = target->mShowOnMaps
                             && target->getPlayerGamePrivilege(Player::ePlayerGamePrivilege_Invisible) == 0;
                if (!shown)
                    visible = false;
            }

            const MapDecoration::Type* type;
#ifdef MINECRAFT_PC
            switch (target->mPlayerIndex % 8)
#else
            switch (target->mPlayerIndex)
#endif
            {
            case 0:
            default:
                type = &MapDecoration::Type::TYPES[0];
                break;
            case 1:
                type = &MapDecoration::Type::TYPES[1];
                break;
            case 2:
                type = &MapDecoration::Type::TYPES[2];
                break;
            case 3:
                type = &MapDecoration::Type::TYPES[3];
                break;
            case 4:
                type = &MapDecoration::Type::TYPES[8];
                break;
            case 5:
                type = &MapDecoration::Type::TYPES[9];
                break;
            case 6:
                type = &MapDecoration::Type::TYPES[10];
                break;
            case 7:
                type = &MapDecoration::Type::TYPES[11];
                break;
            }
            addDecoration(type, target->mLevel, target->getId(), target->mX, target->mY, target->mZ, target->mYRot,
                          visible);
        }
    }

    if (item->isFramed() && mTrackingPosition)
    {
        boost::shared_ptr<ItemFrame> frame = item->getFrame();
        BlockPos pos = frame->getPos();
        addDecoration(&MapDecoration::Type::TYPES[7], player->mLevel, frame->getId(), (double)pos.x, (double)pos.y,
                      (double)pos.z, (double)(frame->mDirection->get2DDataValue() * 90), true);
    }

    if (item->hasTag() && item->getTag()->contains(L"Decorations", Tag::TAG_LIST))
    {
        ListTag* decorations = item->getTag()->getList(L"Decorations", Tag::TAG_COMPOUND);
        for (int i = 0; i < decorations->size(); ++i)
        {
            CompoundTag* decoration = decorations->getCompound(i);
            int id = fjGetDecorationId(decoration);
            if (mDecorationsById.find(id) == mDecorationsById.end())
            {
                addDecoration(MapDecoration::Type::byIcon(decoration->getByte(L"type")), player->mLevel, id,
                              decoration->getDouble(L"x"), 0.0, decoration->getDouble(L"z"),
                              decoration->getDouble(L"rot"), true);
            }
        }
    }

    PIXEndNamedEvent();
}

void MapItemSavedData::mergeInMapData(boost::shared_ptr<MapItemSavedData> data)
{
    int x;
    int y;
    for (x = 0; x < 128; ++x)
        for (y = 0; y < 128; ++y)
        {
            int index = y * 128 + x;
            if (mColors.data[index] == 0 && data->mColors.data[index] != 0)
                mColors.data[index] = data->mColors.data[index];
            setDirty(x, y);
        }
}

std::vector<MapDecoration> MapItemSavedData::fjGetAllDecorations()
{
    std::vector<MapDecoration> result;
    boost::unordered_map<int, MapDecoration>::const_iterator it;
    for (it = mDecorations.begin(); it != mDecorations.end(); ++it)
        result.push_back(it->second);
    for (it = mDecorationsById.begin(); it != mDecorationsById.end(); ++it)
        result.push_back(it->second);
    return result;
}

boost::shared_ptr<Packet> MapItemSavedData::getUpdatePacket(not_null_ptr<ItemInstance> item, Level*,
                                                            boost::shared_ptr<Player> player)
{
    boost::unordered_map<boost::shared_ptr<Player>, boost::shared_ptr<HoldingPlayer>, _PlayerKeyHash,
                         _PlayerKeyEq>::iterator it = mCarriedBy.find(player);
    if (it == mCarriedBy.end())
        return boost::shared_ptr<Packet>();
    boost::shared_ptr<HoldingPlayer> holding = it->second;
    return holding->nextUpdatePacket(item);
}

void MapItemSavedData::load(CompoundTag* tag)
{
    mDimension = tag->getByte(L"dimension");
    mXCenter = tag->getInt(L"xCenter");
    mZCenter = tag->getInt(L"zCenter");
    mScale = tag->getByte(L"scale");
    mScale = (unsigned char)Mth::clamp(mScale, 0, 4);
    if (!tag->contains(L"trackingPosition", Tag::TAG_BYTE))
    {
        mTrackingPosition = true;
    }
    else
    {
        mTrackingPosition = tag->getBoolean(L"trackingPosition");
    }
    mUnlimitedTracking = tag->getBoolean(L"unlimitedTracking");
    int width = tag->getShort(L"width");
    int height = tag->getShort(L"height");
    if (width == 128 && height == 128)
    {
        arrayWithLength<unsigned char> colors = tag->getByteArray(L"colors");
        std::memcpy(mColors.data, colors.data, colors.length);
        return;
    }
    arrayWithLength<unsigned char> colors = tag->getByteArray(L"colors");
    std::memset(mColors.data, 0, mColors.length);
    int xOffset = (128 - width) / 2;
    int yOffset = (128 - height) / 2;
    int x;
    int y;
    for (y = 0; y < height; ++y)
        for (x = 0; x < width; ++x)
            mColors.data[(y + yOffset) * 128 + x + xOffset] = colors.data[y * width + x];
}

CompoundTag* MapItemSavedData::save(CompoundTag* tag)
{
    tag->putByte(L"dimension", mDimension);
    tag->putInt(L"xCenter", mXCenter);
    tag->putInt(L"zCenter", mZCenter);
    tag->putByte(L"scale", mScale);
    tag->putShort(L"width", 128);
    tag->putShort(L"height", 128);
    arrayWithLength<unsigned char> colors(mColors.length, true);
    for (unsigned int i = 0; i < mColors.length; i++)
    {
        colors.data[i] = mColors.data[i];
    }
    tag->putByteArray(L"colors", colors);
    tag->putBoolean(L"trackingPosition", mTrackingPosition);
    tag->putBoolean(L"unlimitedTracking", mUnlimitedTracking);
    return tag;
}

void MapItemSavedData::addTargetDecoration(const not_null_ptr<ItemInstance>& item, const BlockPos& pos,
                                           const std::wstring& id, const MapDecoration::Type* type)
{
    ListTag* decorations;
    if (item->hasTag() && item->getTag()->contains(L"Decorations", Tag::TAG_LIST))
        decorations = item->getTag()->getList(L"Decorations", Tag::TAG_COMPOUND);
    else
    {
        decorations = new ListTag();
        item->addTagElement(L"Decorations", decorations);
    }
    CompoundTag* decoration = new CompoundTag();
    decoration->putByte(L"type", type->getIcon());
    decoration->putString(L"id", id);
    decoration->putDouble(L"x", (double)pos.x);
    decoration->putDouble(L"z", (double)pos.z);
    decoration->putDouble(L"rot", 180.0);
    decorations->add(decoration);
    if (type->hasMapColor())
    {
        CompoundTag* display = static_cast<CompoundTag*>(item->getOrCreateTagElement(ItemInstance::TAG_DISPLAY));
        display->putInt(L"MapColor", type->getMapColor());
    }
}

bool MapItemSavedData::fjIsCompletelyExplored() const
{
    Level* level = gMinecraftServer->getLevel(mDimension);
    unsigned int unexplored = 0;
    for (int x = 0; x < 128; ++x)
    {
        for (int y = 0; y < 128; ++y)
        {
            int scale = 1 << (mScale & 0x3f);
            unsigned char* colors = mColors.data;
            bool generated = level->mDimension->containsChunk((mXCenter + (x - 64) * scale) >> 4,
                                                              (mZCenter + (y - 64) * scale) >> 4);
            if (colors[y * 128 + x] == 0 && generated)
                ++unexplored;
        }
    }
    return unexplored < 0x333;
}

void MapItemSavedData::fjRemoveDecorationByEntityId(int id)
{
    boost::unordered_map<int, MapDecoration>::iterator it = mDecorationsById.find(id);
    if (it != mDecorationsById.end())
    {
        mDecorationsById.erase(it);
    }
}

MapItemSavedData::HoldingPlayer::HoldingPlayer(boost::shared_ptr<Player> player, MapItemSavedData* data)
    : mPlayer(player),
      mDirty(true),
      mMinX(0),
      mMinY(0),
      mMaxX(127),
      mMaxY(127),
      mTick(0),
      mReserved0x20(0),
      mReserved0x24(NULL),
      mReserved0x28(0),
      mStep(0),
      mReserved0x30(false),
      mMapData(data)
{
}

MapItemSavedData::HoldingPlayer::~HoldingPlayer()
{
    delete mReserved0x24;
}

void MapItemSavedData::HoldingPlayer::markDirty(int x, int y)
{
    if (mDirty)
    {
        mMinX = x < mMinX ? x : mMinX;
        mMinY = y < mMinY ? y : mMinY;
        mMaxX = mMaxX < x ? x : mMaxX;
        mMaxY = mMaxY < y ? y : mMaxY;
    }
    else
    {
        mDirty = true;
        mMinX = x;
        mMinY = y;
        mMaxX = x;
        mMaxY = y;
    }
}

boost::shared_ptr<Packet> MapItemSavedData::HoldingPlayer::nextUpdatePacket(not_null_ptr<ItemInstance> item)
{
    std::vector<MapDecoration> decorations = mMapData->fjGetAllDecorations();
    if (mDirty)
    {
        mDirty = false;
        return boost::shared_ptr<Packet>(new ClientboundMapItemDataPacket(
            item->getAuxValue(), mMapData->mScale, mMapData->mTrackingPosition, decorations, mMapData->mColors, mMinX,
            mMinY, mMaxX + 1 - mMinX, mMaxY + 1 - mMinY));
    }
#ifdef MINECRAFT_PC
    ++mTick;
    {
        boost::shared_ptr<Packet> packet(new ClientboundMapItemDataPacket(item->getAuxValue(), mMapData->mScale,
                                                                          mMapData->mTrackingPosition, decorations,
                                                                          mMapData->mColors, 0, 0, 0, 0));
        return packet;
    }
#else
    if (mTick++ % 5 == 0)
    {
        boost::shared_ptr<Packet> packet(new ClientboundMapItemDataPacket(item->getAuxValue(), mMapData->mScale,
                                                                          mMapData->mTrackingPosition, decorations,
                                                                          mMapData->mColors, 0, 0, 0, 0));
        return packet;
    }
#endif
    return boost::shared_ptr<Packet>();
}
