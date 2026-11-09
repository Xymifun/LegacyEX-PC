#include "game/world/generation/StructureFeatureSavedData.hpp"

#include "game/data/CompoundTag.hpp"

template <typename T> std::wstring _toString(T value);

static const std::wstring FEATURES_TAG = L"Features";

StructureFeatureSavedData::StructureFeatureSavedData(const std::wstring& id) : SavedData(id)
{
    mFullTag = new CompoundTag();
}

StructureFeatureSavedData::~StructureFeatureSavedData()
{
    delete mFullTag;
}

void StructureFeatureSavedData::load(CompoundTag* tag)
{
    delete mFullTag;

    if (tag->contains(FEATURES_TAG))
    {
        mFullTag = tag->getCompound(FEATURES_TAG)->copy();
    }
    else
    {
        mFullTag = tag->getCompound(FEATURES_TAG);
    }
}

CompoundTag* StructureFeatureSavedData::save(CompoundTag* tag)
{
    tag->put(FEATURES_TAG, mFullTag->copy());

    return tag;
}

std::wstring StructureFeatureSavedData::createFeatureTagId(int chunkX, int chunkZ)
{
    return L"[" + _toString(chunkX) + L"," + _toString(chunkZ) + L"]";
}

arrayWithLength<unsigned char> StructureFeatureSavedData::getFeatureTag(int chunkX, int chunkZ)
{
    return mFullTag->getByteArray(createFeatureTagId(chunkX, chunkZ));
}

void StructureFeatureSavedData::putFeatureTag(arrayWithLength<unsigned char> featureData, int chunkX, int chunkZ)
{
    mFullTag->putByteArray(createFeatureTagId(chunkX, chunkZ), featureData);
}

CompoundTag* StructureFeatureSavedData::getFullTag()
{
    return mFullTag;
}
