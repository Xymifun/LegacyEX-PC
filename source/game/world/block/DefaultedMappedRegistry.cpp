#include "game/world/block/DefaultedMappedRegistry.hpp"

#include "game/world/block/Block.hpp"
#include "game/world/block/IdMapper.hpp"

DefaultedMappedRegistry<ResourceLocation, Block*>::DefaultedMappedRegistry(ResourceLocation defaultKey)
{
    for (int id = 0; id < BLOCK_COUNT; id++)
    {
        mBlocksById[id] = NULL;
    }
}

Block* DefaultedMappedRegistry<ResourceLocation, Block*>::get(ResourceLocation key)
{
    if (MappedRegistry<ResourceLocation, Block*>::containsKey(key))
    {
        return MappedRegistry<ResourceLocation, Block*>::get(key);
    }

    return mBlocksById[0];
}

void DefaultedMappedRegistry<ResourceLocation, Block*>::registerMapping(int id, ResourceLocation key, Block* value)
{
    registerKey(key, value);

    value->mCachedElementId = id;
    mBlocksById[id] = value;
}

int DefaultedMappedRegistry<ResourceLocation, Block*>::getId(Block* value)
{
    return value->mCachedElementId;
}

Block* DefaultedMappedRegistry<ResourceLocation, Block*>::byId(int id)
{
    return mBlocksById[id];
}

bool DefaultedMappedRegistry<ResourceLocation, Block*>::containsId(int id)
{
    return id >= 0 && id < BLOCK_COUNT && mBlocksById[id] != NULL;
}
