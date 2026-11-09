#include "game/item/DefaultDispenseItemBehavior.hpp"
#include "game/world/LevelEvent.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/Position.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DispenserBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

not_null_ptr<ItemInstance> DefaultDispenseItemBehavior::dispense(BlockSource* blockSource,
                                                                 not_null_ptr<ItemInstance> itemInstance)
{
    eOUTCOME outcome = eOUTCOME_Default;

    not_null_ptr<ItemInstance> result = execute(blockSource, itemInstance, outcome);

    playSound(blockSource, outcome);
    playAnimation(blockSource, blockSource->getBlockState()->getValue(DispenserBlock::FACING), outcome);

    return result;
}

not_null_ptr<ItemInstance> DefaultDispenseItemBehavior::execute(BlockSource* blockSource,
                                                                not_null_ptr<ItemInstance> itemInstance,
                                                                eOUTCOME& outcome)
{
    const Direction* facing = blockSource->getBlockState()->getValue(DispenserBlock::FACING);
    Position* dispensePosition = DispenserBlock::getDispensePosition(blockSource);
    not_null_ptr<ItemInstance> dispensed = itemInstance->split(1);

    spawnItem(blockSource->getWorld(), dispensed, 6, facing, dispensePosition);

    delete dispensePosition;

    outcome = eOUTCOME_Default;

    return itemInstance;
}

void DefaultDispenseItemBehavior::spawnItem(Level* level, not_null_ptr<ItemInstance> itemInstance, int power,
                                            const Direction* direction, Position* position)
{
    double x = position->getX();
    double y = position->getY();
    double z = position->getZ();

    if (direction->getAxis() == Direction::Axis::Y)
    {
        y -= 0.125;
    }
    else
    {
        y -= 0.15625;
    }

    boost::shared_ptr<ItemEntity> itemEntity(new ItemEntity(level, x, y, z, itemInstance));

    double speed = level->mRandom->nextDouble() * 0.1 + 0.2;

    itemEntity->mXd = (double)direction->getStepX() * speed;
    itemEntity->mYd = 0.20000000298023224;
    itemEntity->mZd = (double)direction->getStepZ() * speed;

    itemEntity->mXd += level->mRandom->nextGaussian() * 0.007499999832361937 * (double)power;
    itemEntity->mYd += level->mRandom->nextGaussian() * 0.007499999832361937 * (double)power;
    itemEntity->mZd += level->mRandom->nextGaussian() * 0.007499999832361937 * (double)power;

    level->addEntity(itemEntity);
}

void DefaultDispenseItemBehavior::playSound(BlockSource* blockSource, eOUTCOME outcome)
{
    if (outcome != eOUTCOME_Failure)
    {
        blockSource->getWorld()->levelEvent(LevelEvent::SOUND_DISPENSER_DISPENSE, blockSource->getPos(), 0);
    }
    else
    {
        blockSource->getWorld()->levelEvent(LevelEvent::SOUND_DISPENSER_FAIL, blockSource->getPos(), 0);
    }
}

void DefaultDispenseItemBehavior::playAnimation(BlockSource* blockSource, const Direction* direction, eOUTCOME outcome)
{
    if (outcome != eOUTCOME_Failure)
    {
        blockSource->getWorld()->levelEvent(LevelEvent::PARTICLES_SHOOT, blockSource->getPos(),
                                            getLevelEventDataFrom(direction));
    }
}

int DefaultDispenseItemBehavior::getLevelEventDataFrom(const Direction* direction)
{
    return direction->getStepX() + direction->getStepZ() * 3 + 4;
}
