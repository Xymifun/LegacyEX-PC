#include "game/core/Giant.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

Giant::Giant(Level* level) : Monster(level)
{
    fjDerivedCtorCalls();
    setSize(mBbWidth * 6.0f, mBbHeight * 6.0f);
}

Entity* Giant::create(Level* level)
{
    return new Giant(level);
}

void Giant::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::GIANT);
}

float Giant::getEyeHeight()
{
    return 1.62f * 6.0f;
}

void Giant::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(100.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(50.0);
}

float Giant::getWalkTargetValue(const BlockPos& pos)
{
    return (float)mLevel->getBrightness(pos) - 0.5f;
}

ResourceLocation* Giant::getDefaultLootTable()
{
    return &BuiltInLootTables::GIANT;
}

unsigned long long Giant::GetType()
{
    return EntityType::GIANT;
}
