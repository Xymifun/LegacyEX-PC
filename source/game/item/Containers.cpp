#include "game/item/Containers.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Container.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

Random Containers::RANDOM;

void Containers::dropItemStack(Level* level, double x, double y, double z, not_null_ptr<ItemInstance> itemInstance)
{
    float xOffset = RANDOM.nextFloat() * 0.8f + 0.1f;
    float yOffset = RANDOM.nextFloat() * 0.8f + 0.1f;
    float zOffset = RANDOM.nextFloat() * 0.8f + 0.1f;

    while (!itemInstance->isEmpty())
    {
        not_null_ptr<ItemInstance> split = itemInstance->split(RANDOM.nextInt(21) + 10);
        split->set4JData(itemInstance->get4JData());

        boost::shared_ptr<ItemEntity> itemEntity(
            new ItemEntity(level, x + (double)xOffset, y + (double)yOffset, z + (double)zOffset, split));
        const float speed = 0.05f;
        itemEntity->mXd = (float)RANDOM.nextGaussian() * speed;
        itemEntity->mYd = (float)RANDOM.nextGaussian() * speed + 0.2f;
        itemEntity->mZd = (float)RANDOM.nextGaussian() * speed;
        level->addEntity(itemEntity);
    }
}

void Containers::dropContents(Level* level, double x, double y, double z, boost::shared_ptr<Container> container)
{
    for (unsigned int slot = 0; slot < container->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> itemInstance = container->getItem(slot);
        if (itemInstance->isEmpty())
        {
            continue;
        }

        dropItemStack(level, x, y, z, itemInstance);
        container->setItem(slot, ItemInstance::EMPTY);
    }
}

void Containers::dropContents(Level* level, const BlockPos& pos, boost::shared_ptr<Container> container)
{
    dropContents(level, (double)pos.x, (double)pos.y, (double)pos.z, container);
}

void Containers::dropContents(Level* level, boost::shared_ptr<Entity> entity, boost::shared_ptr<Container> container)
{
    dropContents(level, entity->mX, entity->mY, entity->mZ, container);
}
