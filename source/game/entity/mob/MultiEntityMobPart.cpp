#include "game/entity/mob/MultiEntityMobPart.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/MultiEntityMob.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"

MultiEntityMobPart::MultiEntityMobPart(boost::shared_ptr<MultiEntityMob> parentMob, const std::wstring& name,
                                       float width, float height)
    : Entity(parentMob->getLevel(), true),
      mParentMob(parentMob),
      mName(name)
{
    setSize(width, height);
}

unsigned long long MultiEntityMobPart::GetType()
{
    return EntityType::MULTI_ENTITY_MOB_PART;
}

void MultiEntityMobPart::defineSynchedData()
{
}

bool MultiEntityMobPart::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
    {
        return false;
    }

    return mParentMob.lock()->hurt(boost::static_pointer_cast<MultiEntityMobPart>(shared_from_this()), source, damage);
}

bool MultiEntityMobPart::isPickable()
{
    return true;
}

void MultiEntityMobPart::readAdditionalSaveData(CompoundTag* tag)
{
}

void MultiEntityMobPart::addAdditonalSaveData(CompoundTag* tag)
{
}

unsigned int MultiEntityMobPart::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    boost::shared_ptr<MultiEntityMob> parentMob = mParentMob.lock();

    if (parentMob)
    {
        parentMob->GetInteractTooltip(boost::static_pointer_cast<MultiEntityMobPart>(shared_from_this()), tooltipData);
    }

    return IDS_NONE;
}

bool MultiEntityMobPart::canRide(boost::shared_ptr<Entity> vehicle)
{
    return false;
}

bool MultiEntityMobPart::is(boost::shared_ptr<Entity> entity)
{
    return shared_from_this() == entity || mParentMob.lock() == boost::dynamic_pointer_cast<MultiEntityMob>(entity);
}
