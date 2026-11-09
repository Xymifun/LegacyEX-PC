#ifndef GAME_WORLD_BIOME_BIOMESOURCE_HPP
#define GAME_WORLD_BIOME_BIOMESOURCE_HPP

#include <cstddef>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/CriticalSection.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPos.hpp"

class Biome;
class BiomeCache;
class FjFeatureBiomeRequirements;
class Layer;
class LevelData;
class LevelType;
class Random;
class SuperflatConfig;

class BiomeSource
{
public:
    enum eBiomeGenerationScale
    {
        eBiomeGenerationScale_Small,
        eBiomeGenerationScale_Medium,
        eBiomeGenerationScale_Large
    };

    class LayerOverrideSettings
    {
    public:
        LayerOverrideSettings();
        ~LayerOverrideSettings();

        int getBiomeIndex(int x, int z);

        void SetWidthInChunksToDisplay(int widthInChunks, int extraSections);

        void SetSeed(const std::wstring& seedText);
        void GenerateNewSeed();
        long long GetSeed();

        bool FullyGenerated();

        void SetGenerationScale(BiomeSource::eBiomeGenerationScale scale);
        BiomeSource::eBiomeGenerationScale GetGenerationScale();
        unsigned char GetBiomeScale();

        void SetScaledCentrePosition();
        bool IncreaseScale();
        bool DecreaseScale();

        void SetCentrePosition(int centreXChunk, int centreZChunk);
        int GetCentreXChunk();
        int GetCentreZChunk();

        void RegenerateUp(arrayWithLength<unsigned char> pixels);
        void RegenerateDown(arrayWithLength<unsigned char> pixels);
        void RegenerateLeft(arrayWithLength<unsigned char> pixels);
        void RegenerateRight(arrayWithLength<unsigned char> pixels);

        bool isZoomed();
        int GetZoomedCentreX();
        int GetZoomedCentreZ();
        bool CheckWithinBounds(int& value, int delta);
        void SetZoomedDisplay(int x, int z);
        bool ZoomIn();
        bool ZoomOut();

        bool ScrollUp(arrayWithLength<unsigned char> pixels);
        bool ScrollDown(arrayWithLength<unsigned char> pixels);
        bool ScrollLeft(arrayWithLength<unsigned char> pixels);
        bool ScrollRight(arrayWithLength<unsigned char> pixels);

        unsigned char GetZoomLevel();
        void SetZoomLevel(unsigned char zoomLevel);
        int GetZoomedCentreXPercent();
        int GetZoomedCentreZPercent();

        void NullBiomeSection(int x, int width, int z, int height);
        void RegenerateAllBiomes();
        void RegenerateBiomeSection(int x, int width, int z, int height);
        bool SectionNeedsRegen(int x, int width, int z, int height);
        bool CheckAndRegenerate(int x, int z, bool nullOnly);
        bool RegenerateNearestBiomeSection(int& sectionX, int& sectionZ, bool& atVerticalEdge, bool& atHorizontalEdge);

        int GetBiomeSectionWidth();
        int GetImageWidth();

        BlockPos GetCursorBlockPos(float xFraction, float zFraction);
        Biome* GetBiomeAt(float xFraction, float zFraction);

        arrayWithLength<unsigned char> CreateZoomedImage(unsigned int* biomeMapPixels, int& zoomLevel);
        arrayWithLength<unsigned char> GetBiomeMapPixels(bool& missingBiomes, bool wholeMap);
        void UpdateBiomeMapPixelsInRegion(unsigned int* biomeMapPixels, unsigned int x, unsigned int z,
                                          unsigned int width, unsigned int height, bool& missingBiomes, bool wholeMap);

        Random* mRandom;
        long long mSeed;
        int mWidthInChunksToDisplay;
        int mImageWidth;
        int mHalfImageWidth;
        int mBiomeSectionCount;
        int mBiomeSectionWidth;
        int mHalfBiomeSectionWidth;
        arrayWithLength<Biome*> mBiomes;
        bool mFullyGenerated;
        int mLastGeneratedSection;
        int mChunksPerScroll;
        BiomeSource::eBiomeGenerationScale mGenerationScale;
        int mCentreXChunk;
        int mCentreZChunk;
        unsigned char mZoomLevel;
        int mZoomedCentreX;
        int mZoomedCentreZ;
        OSFastMutex mBiomesCriticalSection;
    };

    class CouldSpawnCache : public boost::unordered_map<ChunkPos, bool, ChunkPosKeyHash, ChunkPosKeyEq,
                                                        std::allocator<std::pair<const ChunkPos, bool> > >
    {
    public:
        CouldSpawnCache(BiomeSource* biomeSource, const FjFeatureBiomeRequirements* requirements);

        bool couldSpawn(int chunkX, int chunkZ);

        BiomeSource* mBiomeSource;
        const FjFeatureBiomeRequirements* mRequirements;
    };

    static const int NUM_BIOMES = 176;
    static const int NUM_BIOME_GROUPS = 8;

    static unsigned char getBiomeGroup(int biomeId);
    static void getFracs(arrayWithLength<int> biomeIndices, float* biomeFracs, float* groupFracs);
    static bool getIsMatch(arrayWithLength<int> biomeIndices, float* biomeFracs, float* groupFracs);
    static long long findSeed(LevelType* levelType);

    BiomeSource();
    BiomeSource(long long seed, LevelType* levelType, SuperflatConfig* levelTypeOptions,
                LayerOverrideSettings* overrideSettings);
    BiomeSource(LevelData* levelData);

    virtual ~BiomeSource();

    virtual Biome* getBiome(const BlockPos& pos);
    virtual Biome* getBiome(const BlockPos& pos, Biome* fallback);
    virtual void getRawBiomeBlock(arrayWithLength<Biome*>& biomes, int x, int z, int width, int height) const;
    virtual void getRawBiomeIndices(arrayWithLength<int>& biomeIndices, int x, int z, int width, int height) const;
    virtual void getBiomeBlock(arrayWithLength<Biome*>& biomes, int x, int z, int width, int height,
                               bool useCache) const;
    virtual unsigned char* getBiomeIndexBlock(int x, int z, int width, int height) const;
    virtual void getBiomeIndexBlock(arrayWithLength<unsigned char>& biomeIndices, int x, int z, int width, int height,
                                    bool useCache) const;
    virtual bool containsOnly(int x, int z, int radius, const std::vector<Biome*>& biomes);
    virtual BlockPos* findBiome(int x, int z, int radius, std::vector<Biome*> biomes, Random* random);
    std::vector<Biome*> getPlayerSpawnBiomes() { return mPlayerSpawnBiomes; }
    virtual bool isFixedBiome();
    virtual Biome* getFixedBiome();

    arrayWithLength<Biome*> getBlockAt(int x, int z);

    float scaleTemp(float temperature, int height) const;

    void update();

    bool fjCouldSpawn(const FjFeatureBiomeRequirements* requirements, int chunkX, int chunkZ);

protected:
    void _init();
    void _init(long long seed, LevelType* levelType, SuperflatConfig* levelTypeOptions, LevelData* levelData,
               LayerOverrideSettings* overrideSettings);

    boost::shared_ptr<Layer> mRawBiomeLayer;
    boost::shared_ptr<Layer> mBiomeLayer;
    BiomeCache* mBiomeCache;
    std::vector<Biome*> mPlayerSpawnBiomes;
    OSFastMutex mCriticalSection;
    std::vector<CouldSpawnCache> mCouldSpawnCaches;
};

#endif
