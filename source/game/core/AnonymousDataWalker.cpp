#include "game/core/AnonymousDataWalker.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixer.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/ListTag.hpp"

AnonymousDataWalker::~AnonymousDataWalker()
{
}

CompoundTag* AnonymousDataWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    if (tag->contains(std::wstring(L"entities"), 9))
    {
        ListTag* entitiesTag = tag->getList(std::wstring(L"entities"), 10);

        for (int i = 0; i < entitiesTag->size(); ++i)
        {
            CompoundTag* entityTag = static_cast<CompoundTag*>(entitiesTag->get(i));

            if (entityTag->contains(std::wstring(L"nbt"), 10))
            {
                entityTag->put(std::wstring(L"nbt"),
                               fixer->update(DataFixTypes::eDataFixTypes_Entity,
                                             entityTag->getCompound(std::wstring(L"nbt")), version));
            }
        }
    }

    if (tag->contains(std::wstring(L"blocks"), 9))
    {
        ListTag* blocksTag = tag->getList(std::wstring(L"blocks"), 10);

        for (int i = 0; i < blocksTag->size(); ++i)
        {
            CompoundTag* blockTag = static_cast<CompoundTag*>(blocksTag->get(i));

            if (blockTag->contains(std::wstring(L"nbt"), 10))
            {
                blockTag->put(std::wstring(L"nbt"),
                              fixer->update(DataFixTypes::eDataFixTypes_BlockEntity,
                                            blockTag->getCompound(std::wstring(L"nbt")), version));
            }
        }
    }

    return tag;
}
