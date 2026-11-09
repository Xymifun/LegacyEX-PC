#include "game/core/ENTITIESONLY_DataWalker.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixer.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/Entity.hpp"

ENTITIESONLY_DataWalker::~ENTITIESONLY_DataWalker()
{
}

CompoundTag* ENTITIESONLY_DataWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    if (tag->contains(L"Entities", Tag::TAG_LIST))
    {
        ListTag* entities = tag->getList(L"Entities", Tag::TAG_COMPOUND);

        for (int i = 0; i < entities->size(); i++)
        {
            if (((CompoundTag*)entities->get(i))->contains(Entity::RIDING_TAG, Tag::TAG_LIST))
            {
                ListTag* passengers = ((CompoundTag*)entities->get(i))->getList(Entity::RIDING_TAG, Tag::TAG_COMPOUND);

                for (int j = 0; j < passengers->size(); j++)
                {
                    passengers->set(
                        j, fixer->update(DataFixTypes::eDataFixTypes_Entity, passengers->getCompound(j), version));
                }
            }

            entities->set(i,
                          fixer->update(DataFixTypes::eDataFixTypes_Entity, (CompoundTag*)entities->get(i), version));
        }
    }

    if (tag->contains(L"TileEntities", Tag::TAG_LIST))
    {
        ListTag* tileEntities = tag->getList(L"TileEntities", Tag::TAG_COMPOUND);

        for (int i = 0; i < tileEntities->size(); i++)
        {
            tileEntities->set(
                i, fixer->update(DataFixTypes::eDataFixTypes_BlockEntity, (CompoundTag*)tileEntities->get(i), version));
        }
    }

    return tag;
}
