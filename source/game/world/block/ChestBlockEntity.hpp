#ifndef GAME_WORLD_BLOCK_CHESTBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_CHESTBLOCKENTITY_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/InteractionObject.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/item/Container.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/block/RandomizableLockableContainerBlockEntity.hpp"

class AbstractContainerMenu;
class Inventory;
class Player;

class ChestBlockEntity : public RandomizableLockableContainerBlockEntity
{
public:
    ChestBlockEntity(bool isBonusChest = false);
    ChestBlockEntity(ChestBlock::Type type, bool isBonusChest);
    virtual ~ChestBlockEntity();

    static BlockEntity* create();
    void _init(bool isBonusChest);

    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual void startOpen(boost::shared_ptr<Player> player);
    virtual void stopOpen(boost::shared_ptr<Player> player);
    virtual bool canPlaceItem(int slot, not_null_ptr<ItemInstance> item);
    virtual NonNullList<not_null_ptr<ItemInstance> > getItems();
    virtual void setItems(NonNullList<not_null_ptr<ItemInstance> > items);
    virtual unsigned int getContainerSize();
    virtual bool isEmpty();
    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual int getMaxStackSize();
    virtual void heyImYourNeighbor(boost::shared_ptr<ChestBlockEntity> neighbour, const Direction* direction);
    virtual void checkNeighbors();
    virtual int getType();
    virtual int getTypeId(bool large);
    virtual AbstractContainerMenu* createMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Player> player);

    virtual void setChanged();
    virtual void load(CompoundTag* tag);
    virtual CompoundTag* save(CompoundTag* tag);
    virtual void clearCache();
    virtual void tick();
    virtual bool triggerEvent(int id, int param);
    virtual void setRemoved();
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual CompoundTag* getUpdateTagForPistonMovement();
    virtual std::wstring getDisplayName();
    virtual unsigned long long GetType();

    static void addDataWalkers(DataFixerUpper* fixer);
    boost::shared_ptr<ChestBlockEntity> getNeighbour();
    void removeNeighbour();
    bool isSameChest(const BlockPos& position);
    boost::weak_ptr<ChestBlockEntity> findAndGreetChestEntity(const Direction* direction);
    void tickRefilledAnimation();
    void startRefilledAnimation();

private:
    friend class BonusChestFeature;
    friend class ServerLevel;

    NonNullList<not_null_ptr<ItemInstance> > mItems;
    bool mIsBonusChest;
    int mRefilledAnimationTicks;
    bool mNeighborsChecked;

public:
    boost::weak_ptr<ChestBlockEntity> mNorthNeighbour;
    boost::weak_ptr<ChestBlockEntity> mEastNeighbour;
    boost::weak_ptr<ChestBlockEntity> mWestNeighbour;
    boost::weak_ptr<ChestBlockEntity> mSouthNeighbour;
    float mOpen;
    float mOpenOld;
    int mOpenCount;

private:
    int mTickCount;
    int mCachedType;
    bool mNeighbourRemoved;
};

#endif
