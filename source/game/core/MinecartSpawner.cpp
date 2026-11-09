#include "game/core/MinecartSpawner.hpp"
#include "game/core/EntityType.hpp"

#include <cwchar>

#include "game/core/OverwritableWalker.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/core/PIXNamedEvents.hpp"

Entity* MinecartSpawner::create(Level* level)
{
    return new MinecartSpawner(level);
}

MinecartSpawner::MinecartSpawner(Level* level) : Minecart(level)
{
    Entity::fjDerivedCtorCalls();
    mSpawner = new MinecartMobSpawner(this);
}

MinecartSpawner::MinecartSpawner(Level* level, double x, double y, double z) : Minecart(level, x, y, z)
{
    Entity::fjDerivedCtorCalls();
    mSpawner = new MinecartMobSpawner(this);
}

MinecartSpawner::~MinecartSpawner()
{
    delete mSpawner;
}

void MinecartSpawner::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity,
                     new OverwritableWalker(EntityType::MINECART_SPAWNER, L"MobSpawner"));
}

const Minecart::Type* MinecartSpawner::getType()
{
    return Minecart::Type::SPAWNER;
}

const BlockState* MinecartSpawner::getDefaultDisplayBlockState()
{
    return Blocks::mob_spawner->defaultBlockState();
}

void MinecartSpawner::readAdditionalSaveData(CompoundTag* tag)
{
    Minecart::readAdditionalSaveData(tag);
    mSpawner->load(tag);
}

void MinecartSpawner::addAdditonalSaveData(CompoundTag* tag)
{
    Minecart::addAdditonalSaveData(tag);
    mSpawner->save(tag);
}

void MinecartSpawner::handleEntityEvent(unsigned char event, int data)
{
    mSpawner->onEventTriggered(event);
}

void MinecartSpawner::tick()
{
    PIXBeginNamedEvent(0.0f, "Minecart spawner tick");
    Minecart::tick();
    mSpawner->tick();
    PIXEndNamedEvent();
}

MinecartSpawner::MinecartMobSpawner* MinecartSpawner::getSpawner()
{
    return mSpawner;
}

unsigned long long MinecartSpawner::GetType()
{
    return EntityType::MINECART_SPAWNER;
}

MinecartSpawner::MinecartMobSpawner::MinecartMobSpawner(MinecartSpawner* minecart)
    : BaseMobSpawner(),
      mMinecart(minecart)
{
}

void MinecartSpawner::MinecartMobSpawner::broadcastEvent(int event)
{
    mMinecart->mLevel->broadcastEntityEvent(mMinecart->shared_from_this(), (unsigned char)event, 0);
}

Level* MinecartSpawner::MinecartMobSpawner::getLevel()
{
    return mMinecart->mLevel;
}

BlockPos MinecartSpawner::MinecartMobSpawner::getPos()
{
    return BlockPos(mMinecart->shared_from_this());
}
