#ifndef GAME_WORLD_BIOME_BIOMECOLORS_HPP
#define GAME_WORLD_BIOME_BIOMECOLORS_HPP

class Biome;
class BlockPos;
class LevelSource;

class BiomeColors
{
public:
    class ColorResolver
    {
    public:
        virtual unsigned int getColor(Biome* biome, const BlockPos& pos) = 0;
    };

    class GrassColorResolver : public ColorResolver
    {
    public:
        virtual unsigned int getColor(Biome* biome, const BlockPos& pos);
    };

    class FoliageColorResolver : public ColorResolver
    {
    public:
        virtual unsigned int getColor(Biome* biome, const BlockPos& pos);
    };

    class WaterColorResolver : public ColorResolver
    {
    public:
        virtual unsigned int getColor(Biome* biome, const BlockPos& pos);
    };

    class SkyColorResolver : public ColorResolver
    {
    public:
        virtual unsigned int getColor(Biome* biome, const BlockPos& pos);
    };

    static unsigned int getAverageColor(LevelSource* level, const BlockPos& pos, ColorResolver* resolver);
    static unsigned int getAverageGrassColor(LevelSource* level, const BlockPos& pos);
    static unsigned int getAverageFoliageColor(LevelSource* level, const BlockPos& pos);
    static unsigned int getAverageWaterColor(LevelSource* level, const BlockPos& pos);
    static unsigned int getAverageSkyColor(LevelSource* level, const BlockPos& pos);
};

#endif
