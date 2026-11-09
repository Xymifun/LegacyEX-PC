#include "game/core/UniqueTypeWalker.hpp"
#include "game/core/EntityType.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixer.hpp"
#include "game/data/DataFixers.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/EntityIO.hpp"

UniqueTypeWalker::UniqueTypeWalker(const ResourceLocation& key) : mKey()
{
    mKey = key;
}

CompoundTag* UniqueTypeWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    if (mKey.toString() == tag->getString(L"id"))
    {
        if (mKey.toString() == EntityIO::getKey(EntityType::VILLAGER).toString()
            && tag->contains(L"Offers", Tag::TAG_COMPOUND))
        {
            CompoundTag* offers = tag->getCompound(L"Offers");
            if (offers->contains(L"Recipes", Tag::TAG_LIST))
            {
                ListTag* recipes = offers->getList(L"Recipes", Tag::TAG_COMPOUND);
                for (int i = 0; i < recipes->size(); ++i)
                {
                    CompoundTag* recipe = recipes->getCompound(i);
                    DataFixers::updateItem(fixer, recipe, version, L"buy");
                    DataFixers::updateItem(fixer, recipe, version, L"buyB");
                    DataFixers::updateItem(fixer, recipe, version, L"sell");
                    recipes->set(i, recipe);
                }
            }
        }

        if (mKey.toString() == L"minecraft:mob_spawner")
        {
            if (tag->contains(L"SpawnPotentials", Tag::TAG_LIST))
            {
                ListTag* potentials = tag->getList(L"SpawnPotentials", Tag::TAG_COMPOUND);
                for (int i = 0; i < potentials->size(); ++i)
                {
                    CompoundTag* potential = potentials->getCompound(i);
                    potential->put(L"Entity", fixer->update(DataFixTypes::eDataFixTypes_Entity,
                                                            potential->getCompound(L"Entity"), version));
                }
            }

            tag->put(L"SpawnData",
                     fixer->update(DataFixTypes::eDataFixTypes_Entity, tag->getCompound(L"SpawnData"), version));
        }
    }

    return tag;
}
