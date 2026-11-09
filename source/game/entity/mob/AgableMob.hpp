#ifndef GAME_ENTITY_MOB_AGABLEMOB_HPP
#define GAME_ENTITY_MOB_AGABLEMOB_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/PathfinderMob.hpp"
#include "game/item/not_null_ptr.hpp"


class CompoundTag;
class EntityDataAccessor_Base;
class EntityTooltipDataHolder;
class ItemInstance;
class Level;

template <class T> class EntityDataAccessor;

class AgableMob : public PathfinderMob
{
public:
    AgableMob(Level* level);
    virtual ~AgableMob() {}

    static void staticCtor();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    bool holdingSpawnEggOfClass(const not_null_ptr<ItemInstance>& itemInstance, unsigned long long type);
    virtual void defineSynchedData();
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner) = 0;
    virtual int getAge();
    virtual void ageUp(int amount, bool forced);
    virtual void ageUp(int amount);
    virtual void setAge(int age);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    virtual void aiStep();
    virtual bool isBaby();
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    virtual void grewUp();
    void internalSetSize(float scale);
    virtual void updateSize(bool baby);
    virtual void setSize(float width, float height);

protected:
    int mAge;
    int mForcedAge;
    int mForcedAgeTimer;
    float mBaseWidth;
    float mBaseHeight;

    static const EntityDataAccessor<bool>* DATA_BABY_ID;
    static const EntityDataAccessor<bool>* DATA_BABY_ID_2;
};

#endif
