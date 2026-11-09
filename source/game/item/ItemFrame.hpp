#ifndef GAME_ITEM_ITEMFRAME_HPP
#define GAME_ITEM_ITEMFRAME_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/HangingEntity.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/item/not_null_ptr.hpp"

class CompoundTag;
class DataFixerUpper;
class Direction;
class EntityDataAccessor_Base;
class EntityTooltipDataHolder;
class ItemDataWalker;
class ItemInstance;
class Level;
class Player;

class ItemFrame : public HangingEntity
{
public:
    ItemFrame(Level* level);
    ItemFrame(Level* level, const BlockPos& pos, const Direction* direction);
    virtual ~ItemFrame();

    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    static void staticCtor();
    void _init();
    virtual void defineSynchedData();
    virtual float getPickRadius();
    not_null_ptr<ItemInstance> getItem();
    void setItem(not_null_ptr<ItemInstance> item, bool updateNeighbours);
    void setItem(not_null_ptr<ItemInstance> item);
    virtual bool hurt(const DamageSource* source, float damage);
    virtual int getWidth();
    virtual int getHeight();
    virtual bool shouldRenderAtSqrDistance(double distanceSqr);
    virtual void dropItem(boost::shared_ptr<Entity> breaker);
    void removeFramedMap(not_null_ptr<ItemInstance> item);
    virtual void dropItem(boost::shared_ptr<Entity> breaker, bool dropSelf);
    virtual void playPlacementSound();
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    int getRotation();
    void setRotation(int rotation, bool update);
    void setRotation(int rotation);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    static boost::shared_ptr<ItemFrame> fjGetItemFrameForPos(Level* level, const BlockPos& pos);
    bool fjTryAwardMapRoom(const boost::shared_ptr<Player>& player);
    virtual bool interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    virtual unsigned int GetHitTooltip(const EntityTooltipDataHolder& holder);
    virtual int getAnalogOutput();
    virtual unsigned long long GetType();

    static const EntityDataAccessor<not_null_ptr<ItemInstance> >* DATA_ITEM;
    static const EntityDataAccessor<int>* DATA_ROTATION;

private:
    float mDropChance;
};

#endif
