#ifndef GAME_WORLD_BIOME_BIOMEDECORATOR_HPP
#define GAME_WORLD_BIOME_BIOMEDECORATOR_HPP

#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/KelpFeature.hpp"

class Biome;
class BlueIceFeature;
class BushFeature;
class CactusFeature;
class ClayFeature;
class CoralCrustFeature;
class CoralFeature;
class CoralHangFeature;
class FlowerFeature;
class HugeMushroomFeature;
class IcebergFeature;
class OreFeature;
class ReedsFeature;
class SandFeature;
class SeaAnemoneFeature;
class SeaPickleFeature;
class SeagrassFeature;
class WaterlilyFeature;
class Level;
class Random;
class CustomizableSourceSettings;

class BiomeDecorator
{
public:
    BiomeDecorator();
    virtual ~BiomeDecorator();
    virtual void decorate(Level* level, Random* random, Biome* biome, const BlockPos& origin);
    virtual void decorate(Biome* biome, Level* level, Random* random);
    virtual void decorateOres(Level* level, Random* random);

    void decorateDepthSpan(Level* level, Random* random, int count, Feature* feature, int minY, int maxY);
    void decorateDepthAverage(Level* level, Random* random, int count, Feature* feature, int centreY, int radius);

    bool decorating;
    BlockPos position;
    CustomizableSourceSettings* settings;
    ClayFeature* clayGen;
    SandFeature* sandGen;
    SandFeature* gravelAsSandGen;
    OreFeature* dirtGen;
    OreFeature* gravelGen;
    OreFeature* graniteGen;
    OreFeature* dioriteGen;
    OreFeature* andesiteGen;
    OreFeature* coalGen;
    OreFeature* ironGen;
    OreFeature* goldGen;
    OreFeature* redstoneGen;
    OreFeature* diamondGen;
    OreFeature* lapisGen;
    FlowerFeature* flowerGen;
    BushFeature* brownMushroomGen;
    BushFeature* redMushroomGen;
    HugeMushroomFeature* bigMushroomGen;
    ReedsFeature* reedGen;
    CactusFeature* cactusGen;
    WaterlilyFeature* waterlilyGen;
    KelpFeature* kelpGen;
    CoralCrustFeature* coralCrustGen;
    CoralFeature* coralGen;
    CoralHangFeature* coralHangGen;
    SeaAnemoneFeature* seaAnemoneGen;
    SeagrassFeature* seagrassGen;
    SeaPickleFeature* seaPickleGen;
    IcebergFeature* icebergGen;
    BlueIceFeature* blueIceGen;
    int waterlilyPerChunk;
    int treesPerChunk;
    float extraTreeChance;
    int flowersPerChunk;
    int grassPerChunk;
    int deadBushPerChunk;
    int mushroomsPerChunk;
    int reedsPerChunk;
    int cactiPerChunk;
    int sandPerChunk;
    int sandPerChunk2;
    int clayPerChunk;
    int bigMushroomsPerChunk;
    int kelpPerChunk;
    int seaGrassPerChunk;
    int seagrassPerChunk;
    bool generateLakes;
};

#endif
