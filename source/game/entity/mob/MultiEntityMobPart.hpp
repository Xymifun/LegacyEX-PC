#ifndef GAME_ENTITY_MOB_MULTIENTITYMOBPART_HPP
#define GAME_ENTITY_MOB_MULTIENTITYMOBPART_HPP

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/entity/mob/Entity.hpp"

class CompoundTag;
class DamageSource;
class EntityTooltipDataHolder;
class MultiEntityMob;

class MultiEntityMobPart : public Entity
{
public:
    MultiEntityMobPart(boost::shared_ptr<MultiEntityMob> parentMob, const std::wstring& name, float width,
                       float height);

    virtual unsigned long long GetType();
    virtual void defineSynchedData();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual bool isPickable();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual bool canRide(boost::shared_ptr<Entity> vehicle);
    virtual bool is(boost::shared_ptr<Entity> entity);

    boost::weak_ptr<MultiEntityMob> mParentMob;
    std::wstring mName;
};

#endif
