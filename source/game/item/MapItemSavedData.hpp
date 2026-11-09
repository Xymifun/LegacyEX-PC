#ifndef GAME_ITEM_MAPITEMSAVEDDATA_HPP
#define GAME_ITEM_MAPITEMSAVEDDATA_HPP
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "game/data/SavedData.hpp"
#include "game/core/MapDecoration.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/network/ClientboundMapItemDataPacket.hpp"
class BlockPos;
class CompoundTag;
class ItemInstance;
class Level;
class MapExtendingRecipe;
class Player;
class ClientboundMapItemDataPacket;
class MapItemSavedData : public SavedData
{
    friend class ItemFrame;

public:
    using SavedData::setDirty;

    class HoldingPlayer
    {
    public:
        HoldingPlayer(boost::shared_ptr<Player> player, MapItemSavedData* data);
        ~HoldingPlayer();
        boost::shared_ptr<Packet> nextUpdatePacket(not_null_ptr<ItemInstance> item);
        void markDirty(int x, int y);

    private:
        friend class MapItemSavedData;
        friend class MapItem;

        boost::shared_ptr<Player> mPlayer;
        bool mDirty;
        int mMinX;
        int mMinY;
        int mMaxX;
        int mMaxY;
        int mTick;
        int mReserved0x20;
        unsigned char* mReserved0x24;
        int mReserved0x28;
        int mStep;
        bool mReserved0x30;
        MapItemSavedData* mMapData;
    };

private:
    friend class ClientboundMapItemDataPacket;

public:
    MapItemSavedData(const std::wstring& id);
    virtual ~MapItemSavedData();
    void setOrigin(double x, double z, int scale);
    boost::shared_ptr<HoldingPlayer> getHoldingPlayer(boost::shared_ptr<Player> player);
    void setDirty(int x, int y);
    int fjEnumerateAxisPoint(int point);
    int fjGetDecorationId(CompoundTag* tag);
    void addDecoration(const MapDecoration::Type* type, Level* level, int id, double x, double y, double z,
                       double rotation, bool visible);
    void tickCarriedBy(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item);
    void mergeInMapData(boost::shared_ptr<MapItemSavedData> data);
    std::vector<MapDecoration> fjGetAllDecorations();
    boost::shared_ptr<Packet> getUpdatePacket(not_null_ptr<ItemInstance> item, Level* level,
                                              boost::shared_ptr<Player> player);
    virtual void load(CompoundTag* tag);
    virtual CompoundTag* save(CompoundTag* tag);
    static void addTargetDecoration(const not_null_ptr<ItemInstance>& item, const BlockPos& pos, const std::wstring& id,
                                    const MapDecoration::Type* type);
    bool fjIsCompletelyExplored() const;
    void fjRemoveDecorationByEntityId(int id);

private:
    friend class MapItem;
    friend class MapExtendingRecipe;

    int mXCenter;
    int mZCenter;
    signed char mDimension;
    bool mTrackingPosition;
    bool mUnlimitedTracking;
    unsigned char mScale;

public:
    arrayWithLength<unsigned char> mColors;

private:
    std::vector<boost::shared_ptr<HoldingPlayer> > mHoldingPlayers;
    boost::unordered_map<boost::shared_ptr<Player>, boost::shared_ptr<HoldingPlayer>, _PlayerKeyHash, _PlayerKeyEq>
        mCarriedBy;
    boost::unordered_map<int, MapDecoration> mDecorations;
    long long mLastUpdateTime;
    boost::unordered_map<int, MapDecoration> mDecorationsById;
};
#endif
