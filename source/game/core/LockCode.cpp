#include "game/core/LockCode.hpp"

#include "game/data/CompoundTag.hpp"

LockCode* LockCode::NO_LOCK = new LockCode(L"");

static const std::wstring LOCK_TAG = L"Lock";

LockCode::LockCode(const std::wstring& key) : mKey(key)
{
}

LockCode::~LockCode()
{
}

bool LockCode::isEmpty()
{
    return mKey.compare(L"") == 0 || mKey.empty();
}

std::wstring LockCode::getKey()
{
    return mKey;
}

void LockCode::addTag(CompoundTag* tag)
{
    tag->putString(LOCK_TAG, mKey);
}

LockCode* LockCode::fromTag(CompoundTag* tag)
{
    if (tag->contains(LOCK_TAG, Tag::TAG_STRING))
    {
        return new LockCode(tag->getString(LOCK_TAG));
    }

    return LockCode::NO_LOCK;
}
