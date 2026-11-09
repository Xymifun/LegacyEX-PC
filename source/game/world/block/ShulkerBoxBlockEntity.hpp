#ifndef GAME_WORLD_BLOCK_SHULKERBOXBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_SHULKERBOXBLOCKENTITY_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/InteractionObject.hpp"
#include "game/item/Container.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/WorldlyContainer.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/RandomizableLockableContainerBlockEntity.hpp"

class AbstractContainerMenu;
class AABB;
class BlockEntity;
class BlockState;
class CompoundTag;
class DataFixerUpper;
class Inventory;
class Packet;
class Player;

class ShulkerBoxBlockEntity : public RandomizableLockableContainerBlockEntity, public WorldlyContainer
{
public:
    enum AnimationStatus
    {
        ANIMATION_CLOSED,
        ANIMATION_OPENING,
        ANIMATION_OPENED,
        ANIMATION_CLOSING
    };

    static const int COLUMNS;
    static const int ROWS;
    static const float LID_HEIGHT_FACTOR;
    static const float LID_ROTATION_DEGREES;

    ShulkerBoxBlockEntity(const DyeColor* color);
    ShulkerBoxBlockEntity();
    virtual ~ShulkerBoxBlockEntity();

    static BlockEntity* create();
    void _init(const DyeColor* color);
    virtual void startOpen(boost::shared_ptr<Player> player);
    virtual void stopOpen(boost::shared_ptr<Player> player);
    virtual void clearContent();
    virtual NonNullList<not_null_ptr<ItemInstance> > getItems();
    virtual void setItems(NonNullList<not_null_ptr<ItemInstance> > items);
    virtual unsigned int getContainerSize();
    virtual int getMaxStackSize();
    virtual AbstractContainerMenu* createMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Player> player);
    virtual int getTypeId(bool large);
    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual bool isEmpty();
    virtual arrayWithLength<int> getSlotsForFace(const Direction* face);
    virtual bool canPlaceItemThroughFace(int slot, not_null_ptr<ItemInstance> item, const Direction* face);
    virtual bool canTakeItemThroughFace(int slot, not_null_ptr<ItemInstance> item, const Direction* face);

    virtual void setChanged();
    int getAnimationStatus();
    void setDestroyedByCreativePlayer(bool value);
    bool isCleared();
    bool isDestroyedByCreativePlayer();
    bool shouldDrop();
    CompoundTag* saveToTag(CompoundTag* tag);
    float getProgress(float partialTick);
    AABB* getBoundingBox(const Direction* direction);
    AABB* getBoundingBox(const BlockState* state);
    AABB* getTopBoundingBox(const Direction* direction);
    void moveCollidedEntities();
    void updateAnimation();
    virtual void tick();
    virtual bool triggerEvent(int id, int param);
    static void addDataWalkers(DataFixerUpper* fixer);
    void loadFromTag(CompoundTag* tag);
    virtual void load(CompoundTag* tag);
    virtual CompoundTag* save(CompoundTag* tag);
    const DyeColor* getColor();
    virtual boost::shared_ptr<Packet> getUpdatePacket();
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual unsigned long long GetType();
    ShulkerBoxBlockEntity& operator=(const ShulkerBoxBlockEntity& other);

private:
    static arrayWithLength<int> SLOTS;

    NonNullList<not_null_ptr<ItemInstance> > mItems;
    bool mCleared;
    int mOpenCount;
    int mAnimationStatus;
    float mProgress;
    float mProgressOld;
    const DyeColor* mColor;
    bool mDestroyedByCreativePlayer;
};

#endif
