#include "game/core/SpawnData.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/util/ResourceLocation.hpp"

static const std::wstring EMPTY_STRING = L"";

SpawnData::SpawnData() : WeighedRandomItem(1), mTag(new CompoundTag())
{
    mTag->putString(std::wstring(L"id"), EMPTY_STRING);
}

SpawnData::SpawnData(CompoundTag* tag)
    : WeighedRandomItem(tag->contains(std::wstring(L"Weight"), 99) ? tag->getInt(std::wstring(L"Weight")) : 1)
{
    mTag = tag->getCompound(std::wstring(L"Entity"))->copy();
}

SpawnData::SpawnData(int weight, CompoundTag* entityTag) : WeighedRandomItem(weight), mTag(entityTag->copy())
{
}

SpawnData::~SpawnData()
{
    delete mTag;
}

CompoundTag* SpawnData::save()
{
    CompoundTag* tag = new CompoundTag();

    if (!mTag->contains(std::wstring(L"id"), 8))
    {
        mTag->putString(std::wstring(L"id"), EMPTY_STRING);
    }
    else
    {
        if (mTag->getString(std::wstring(L"id")).find(L':') == std::wstring::npos)
        {
            mTag->putString(std::wstring(L"id"), ResourceLocation(mTag->getString(std::wstring(L"id"))).toString());
        }
    }

    tag->put(std::wstring(L"Entity"), mTag->copy());
    tag->putInt(std::wstring(L"Weight"), weight);

    return tag;
}

CompoundTag* SpawnData::getTag()
{
    return mTag;
}
