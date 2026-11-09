#include "game/core/KeyedDataWalker.hpp"
#include "game/core/EntityType.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/world/block/BlockEntity.hpp"

KeyedDataWalker::KeyedDataWalker(unsigned long long type) : mKey()
{
    if ((EntityType::ENTITY & type) == EntityType::ENTITY)
    {
        mKey = EntityIO::getKey(type);
    }
    else if ((type & BlockEntityType::BLOCK_ENTITY) == BlockEntityType::BLOCK_ENTITY)
    {
        mKey = BlockEntity::getKey(type);
    }
    else
    {
        mKey = ResourceLocation(std::wstring(L""));
    }
}

CompoundTag* KeyedDataWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    if (ResourceLocation(tag->getString(L"id")).equals(&mKey))
    {
        tag = doWalk(fixer, tag, version);
    }

    return tag;
}
