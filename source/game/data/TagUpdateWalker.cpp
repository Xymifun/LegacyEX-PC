#include "game/data/TagUpdateWalker.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixer.hpp"

TagUpdateWalker::TagUpdateWalker(const std::wstring& name, unsigned char tagType,
                                 DataFixTypes::EDataFixTypes dataFixType)
    : mName(name),
      mTagType(tagType),
      mDataFixType(dataFixType)
{
}

CompoundTag* TagUpdateWalker::walk(DataFixer* fixer, CompoundTag* tag, int version)
{
    if (tag->contains(mName, mTagType))
    {
        tag->put(mName, fixer->update(mDataFixType, tag->getCompound(mName), version));
    }

    return tag;
}
