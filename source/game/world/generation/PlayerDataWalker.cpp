#include "game/world/generation/PlayerDataWalker.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixer.hpp"
#include "game/data/DataFixers.hpp"

PlayerDataWalker::PlayerDataWalker(std::vector<std::wstring>* itemFields) : mItemFields(itemFields)
{
}

PlayerDataWalker::~PlayerDataWalker()
{
    delete mItemFields;
}

CompoundTag* PlayerDataWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    for (std::vector<std::wstring>::iterator it = mItemFields->begin(); it != mItemFields->end(); ++it)
    {
        DataFixers::updateItems(fixer, tag, version, *it);
    }

    if (tag->contains(std::wstring(L"ShoulderEntityLeft"), 10))
    {
        tag->put(std::wstring(L"ShoulderEntityLeft"),
                 fixer->update(DataFixTypes::eDataFixTypes_Entity,
                               tag->getCompound(std::wstring(L"ShoulderEntityLeft")), version));
    }

    if (tag->contains(std::wstring(L"ShoulderEntityRight"), 10))
    {
        tag->put(std::wstring(L"ShoulderEntityRight"),
                 fixer->update(DataFixTypes::eDataFixTypes_Entity,
                               tag->getCompound(std::wstring(L"ShoulderEntityRight")), version));
    }

    return tag;
}
