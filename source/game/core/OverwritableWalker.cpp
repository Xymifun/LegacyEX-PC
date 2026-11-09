#include "game/core/OverwritableWalker.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixer.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/util/ResourceLocation.hpp"

OverwritableWalker::OverwritableWalker(unsigned long long type, const std::wstring& name) : mType(type), mName(name)
{
}

CompoundTag* OverwritableWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    const std::wstring oldId(tag->getString(L"id"));
    ResourceLocation id(oldId);

    if (EntityIO::getKey(mType).equals(&id))
    {
        tag->putString(L"id", mName);
        fixer->update(DataFixTypes::eDataFixTypes_BlockEntity, tag, version);
        tag->putString(L"id", oldId);
    }

    return tag;
}
