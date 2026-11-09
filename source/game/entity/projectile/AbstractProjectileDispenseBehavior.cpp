#include "game/entity/projectile/AbstractProjectileDispenseBehavior.hpp"
#include "game/world/LevelEvent.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/Position.hpp"
#include "game/core/EntityType.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/projectile/Projectile.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DispenserBlock.hpp"

not_null_ptr<ItemInstance> AbstractProjectileDispenseBehavior::execute(BlockSource* blockSource,
                                                                       not_null_ptr<ItemInstance> itemInstance,
                                                                       DefaultDispenseItemBehavior::eOUTCOME& outcome)
{
    Level* level = blockSource->getWorld();

    if (level->countInstanceOf(EntityType::PROJECTILE, false, NULL, NULL) >= MAX_PROJECTILES)
    {
        return DefaultDispenseItemBehavior::execute(blockSource, itemInstance, outcome);
    }

    Position* position = DispenserBlock::getDispensePosition(blockSource);
    const Direction* direction = blockSource->getBlockState()->getValue(DispenserBlock::FACING);
    boost::shared_ptr<Projectile> projectile = getProjectile(level, position, itemInstance);
    delete position;

    projectile->shoot(direction->getStepX(), direction->getStepY() + 0.1f, direction->getStepZ(), getPower(),
                      getUncertainty());
    level->addEntity(boost::dynamic_pointer_cast<Entity>(projectile));
    itemInstance->split(1);
    return itemInstance;
}

void AbstractProjectileDispenseBehavior::playSound(BlockSource* blockSource,
                                                   DefaultDispenseItemBehavior::eOUTCOME outcome)
{
    if (outcome != DefaultDispenseItemBehavior::eOUTCOME_Failure)
    {
        blockSource->getWorld()->levelEvent(LevelEvent::SOUND_DISPENSER_PROJECTILE_LAUNCH, blockSource->getPos(), 0);
    }
}

float AbstractProjectileDispenseBehavior::getUncertainty()
{
    return 6.0f;
}

float AbstractProjectileDispenseBehavior::getPower()
{
    return 1.1f;
}
