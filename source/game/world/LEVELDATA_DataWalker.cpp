#include "game/world/LEVELDATA_DataWalker.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixer.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/Entity.hpp"

LEVELDATA_DataWalker::~LEVELDATA_DataWalker()
{
}

CompoundTag* LEVELDATA_DataWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    if (tag->contains(L"Level", Tag::TAG_COMPOUND))
    {
        CompoundTag* level = tag->getCompound(L"Level");

        if (level->contains(L"Entities", Tag::TAG_LIST))
        {
            ListTag* entities = level->getList(L"Entities", Tag::TAG_COMPOUND);

            for (int i = 0; i < entities->size(); i++)
            {
                if (((CompoundTag*)entities->get(i))->contains(Entity::RIDING_TAG, Tag::TAG_LIST))
                {
                    ListTag* passengers =
                        ((CompoundTag*)entities->get(i))->getList(Entity::RIDING_TAG, Tag::TAG_COMPOUND);

                    for (int j = 0; j < passengers->size(); j++)
                    {
                        passengers->set(
                            j, fixer->update(DataFixTypes::eDataFixTypes_Entity, passengers->getCompound(j), version));
                    }
                }

                entities->set(
                    i, fixer->update(DataFixTypes::eDataFixTypes_Entity, (CompoundTag*)entities->get(i), version));
            }
        }

        if (level->contains(L"TileEntities", Tag::TAG_LIST))
        {
            ListTag* tileEntities = level->getList(L"TileEntities", Tag::TAG_COMPOUND);

            for (int i = 0; i < tileEntities->size(); i++)
            {
                tileEntities->set(i, fixer->update(DataFixTypes::eDataFixTypes_BlockEntity,
                                                   (CompoundTag*)tileEntities->get(i), version));
            }
        }
    }

    return tag;
}
