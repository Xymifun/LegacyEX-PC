#ifndef PLATFORM_PC_PCMODSTRUCTURES_HPP
#define PLATFORM_PC_PCMODSTRUCTURES_HPP

#include <string>
#include <vector>

#include "game/world/biome/Biome.hpp"

struct PcNativeRegister;
class BlockPos;
class ChunkPos;
class ChunkPrimer;
class Level;
class MobCategory;
class Random;
class StructureFeature;

struct PcModFeature
{
    std::wstring key;
    StructureFeature* feature;
    void* (*mobs)(void* feature, void* category, const void* pos);
};
typedef std::vector<PcModFeature> PcModFeatures;

PcModFeatures* PcModStructuresMake(Level* level, int dimension, void* levelSource);
void PcModStructuresFree(PcModFeatures* list);
void PcModStructuresApply(PcModFeatures* list, Level* level, int chunkX, int chunkZ, ChunkPrimer* primer);
void PcModStructuresPostProcess(PcModFeatures* list, Level* level, Random& random, ChunkPos* pos);
StructureFeature* PcModStructuresFind(PcModFeatures* list, const std::wstring& name);
std::vector<Biome::MobSpawnerData*>* PcModStructuresMobs(PcModFeatures* list, MobCategory* category, const BlockPos& pos);

void PcModStructuresFillRegister(PcNativeRegister* reg);

#endif
