#include "game/entity/mob/ShoulderRidingEntity.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

ShoulderRidingEntity::ShoulderRidingEntity(Level* level) : TamableAnimal(level), mShoulderTick(0)
{
}

bool ShoulderRidingEntity::setEntityOnShoulder(boost::shared_ptr<Player> player)
{
    CompoundTag* tag = new CompoundTag();
    tag->putString(L"id", getEncodeId());
    saveWithoutId(tag);

    if (player->setEntityOnShoulder(tag))
    {
        mLevel->removeEntity(shared_from_this());
        return true;
    }

    return false;
}

void ShoulderRidingEntity::tick()
{
    ++mShoulderTick;
    Mob::tick();
}

bool ShoulderRidingEntity::canSitOnShoulder()
{
    return mShoulderTick > 100;
}
