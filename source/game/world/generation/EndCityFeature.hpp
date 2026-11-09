#ifndef GAME_WORLD_GENERATION_ENDCITYFEATURE_HPP
#define GAME_WORLD_GENERATION_ENDCITYFEATURE_HPP

#include <string>

#include "game/world/generation/StructureFeature.hpp"
#include "game/world/generation/StructureStart.hpp"

class BlockPos;
class Level;
class Random;
class TheEndLevelSource;

class EndCityFeature : public StructureFeature
{
public:
    class EndCityStart : public StructureStart
    {
    public:
        EndCityStart(Level* level, TheEndLevelSource* source, Random& random, int chunkX, int chunkZ);
        void create(Level* level, TheEndLevelSource* source, Random& random, int chunkX, int chunkZ);
        virtual bool isValid();
        virtual EStructureStart GetType();

    private:
        bool mValid;
    };

    EndCityFeature(TheEndLevelSource* source);
    virtual std::wstring getFeatureName();
    virtual bool isFeatureChunk(int chunkX, int chunkZ, bool flatWorld);
    static int getYPositionForFeature(int chunkX, int chunkZ, TheEndLevelSource* source);
    virtual StructureStart* createStructureStart(int chunkX, int chunkZ);
    virtual BlockPos* getNearestGeneratedFeature(Level* level, const BlockPos& pos, bool skipExistingChunks);

private:
    TheEndLevelSource* mSource;
};

#endif
