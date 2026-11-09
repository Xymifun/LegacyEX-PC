#ifndef GAME_WORLD_GENERATION_STRUCTUREFEATURESAVEDDATA_HPP
#define GAME_WORLD_GENERATION_STRUCTUREFEATURESAVEDDATA_HPP

#include <string>

#include "game/data/SavedData.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class CompoundTag;

class StructureFeatureSavedData : public SavedData
{
public:
    StructureFeatureSavedData(const std::wstring& id);

    virtual ~StructureFeatureSavedData();

    virtual void load(CompoundTag* tag);
    virtual CompoundTag* save(CompoundTag* tag);

    static std::wstring createFeatureTagId(int chunkX, int chunkZ);

    arrayWithLength<unsigned char> getFeatureTag(int chunkX, int chunkZ);
    void putFeatureTag(arrayWithLength<unsigned char> featureData, int chunkX, int chunkZ);
    CompoundTag* getFullTag();

private:
    CompoundTag* mFullTag;
};

#endif
