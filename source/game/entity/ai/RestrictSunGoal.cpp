#include "game/entity/ai/RestrictSunGoal.hpp"

#include "game/core/EquipmentSlot.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Material.hpp"

RestrictSunGoal::RestrictSunGoal(PathfinderMob* mob) : Goal(), mMob(mob)
{
}

RestrictSunGoal::~RestrictSunGoal()
{
}

bool RestrictSunGoal::canUse()
{
    return mMob->mLevel->isDay() && mMob->getItemSlot(EquipmentSlot::HEAD)->isEmpty()
           && !mMob->isUnderLiquid(Material::WATER);
}

void RestrictSunGoal::start()
{
    static_cast<GroundPathNavigation*>(mMob->getNavigation())->setAvoidSun(true);
}

void RestrictSunGoal::stop()
{
    static_cast<GroundPathNavigation*>(mMob->getNavigation())->setAvoidSun(false);
}
