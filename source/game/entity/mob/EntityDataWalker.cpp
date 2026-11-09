#include "game/entity/mob/EntityDataWalker.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixer.hpp"
#include "game/data/ListTag.hpp"

EntityDataWalker::~EntityDataWalker()
{
}

CompoundTag* EntityDataWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    if (tag->contains(std::wstring(L"Passengers"), 9))
    {
        ListTag* passengers = tag->getList(std::wstring(L"Passengers"), 10);

        for (int i = 0; i < passengers->size(); i++)
        {
            passengers->set(i, fixer->update(DataFixTypes::eDataFixTypes_Entity, passengers->getCompound(i), version));
        }
    }

    return tag;
}
