#include "game/world/generation/VillageFeature.hpp"

#include <cwchar>
#include <vector>

#include "game/core/BoundingBox.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/world/generation/VillagePieces.hpp"

std::vector<Biome*> VillageFeature::allowedBiomes;

VillageFeature::VillageFeature(int size) : StructureFeature()
{
    _init(size);
}

VillageFeature::VillageFeature(SuperflatConfig* config, int size) : StructureFeature()
{
    FlatGeneratorInfo generatorInfo(config, false);
    _init(size);
    mVillageSize = generatorInfo.getVillageSize();
    mSpacing = generatorInfo.getVillageDistance();
}

void VillageFeature::staticCtor()
{
    allowedBiomes.push_back(Biome::PLAINS);
    allowedBiomes.push_back(Biome::DESERT);
    allowedBiomes.push_back(Biome::SAVANNA);
    allowedBiomes.push_back(Biome::TAIGA);
    allowedBiomes.push_back(Biome::COLD_TAIGA);
    allowedBiomes.push_back(Biome::ICE_PLAINS);
}

void VillageFeature::_init(int size)
{
    mVillageSize = 0;
    mSpacing = 0x20;
    mSeparation = 8;
    mSize = size << 4;
}

std::wstring VillageFeature::getFeatureName()
{
    return std::wstring(L"Village");
}

bool VillageFeature::isFeatureChunk(int chunkX, int chunkZ, bool flatWorld)
{
    int spacing = mSpacing;
    if (!flatWorld)
        spacing = 16;
    int originalX = chunkX;
    int originalZ = chunkZ;
    if (chunkX < 0)
        chunkX += 1 - spacing;
    if (chunkZ < 0)
        chunkZ += 1 - spacing;
    int centerX = chunkX / spacing;
    int centerZ = chunkZ / spacing;
    Random* chunkRandom = level->getRandomFor(centerX, centerZ, 10387312);
    centerX *= spacing;
    centerZ *= spacing;
    centerX += chunkRandom->nextInt(spacing - mSeparation);
    centerZ += chunkRandom->nextInt(spacing - mSeparation);
    chunkX = originalX;
    chunkZ = originalZ;
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if ((options != NULL && options->isFeatureChunk(chunkX, chunkZ, eFeatureType_Village, NULL))
        || (chunkX == centerX && chunkZ == centerZ))
    {
        if (level->getBiomeSource()->containsOnly(chunkX * 16 + 8, chunkZ * 16 + 8, 0, allowedBiomes))
        {
            return true;
        }
    }
    return false;
}

BlockPos* VillageFeature::getNearestGeneratedFeature(Level* levelIn, const BlockPos& pos, bool skipExistingChunks)
{
    level = levelIn;
    return StructureFeature::findNearestFeaturePositionBySpacing(levelIn, this, pos, mSpacing, mSeparation, 10387312,
                                                                 false, 100, skipExistingChunks);
}

StructureStart* VillageFeature::createStructureStart(int chunkX, int chunkZ)
{
    gApp.AddTerrainFeaturePosition((_eTerrainFeatureType)3, chunkX, chunkZ);
    return new VillageStart(level, random, chunkX, chunkZ, mVillageSize, mSize);
}

VillageFeature::VillageStart::VillageStart(Level* levelIn, Random& random, int chunkX, int chunkZ, int size,
                                           int distance)
    : StructureStart(chunkX, chunkZ),
      mValid(false),
      mDistance(distance)
{
    std::list<VillagePieces::PieceWeight*>* pieceSet = VillagePieces::createPieceSet(random, size);
    VillagePieces::StartPiece* startPiece = new VillagePieces::StartPiece(
        levelIn->getBiomeSource(), 0, random, chunkX * 16 + 2, chunkZ * 16 + 2, pieceSet, size, levelIn);
    mPieces.push_back(startPiece);
    startPiece->addChildren(startPiece, &mPieces, random);

    std::vector<StructurePiece*>& roadPieces = startPiece->mRoadPieces;
    std::vector<StructurePiece*>& pendingPieces = startPiece->mPendingPieces;
    while (!roadPieces.empty() || !pendingPieces.empty())
    {
        if (roadPieces.empty())
        {
            int index = random.nextInt((int)pendingPieces.size());
            std::vector<StructurePiece*>::iterator it = pendingPieces.begin() + index;
            StructurePiece* piece = *it;
            pendingPieces.erase(it);
            piece->addChildren(startPiece, &mPieces, random);
        }
        else
        {
            int index = random.nextInt((int)roadPieces.size());
            std::vector<StructurePiece*>::iterator it = roadPieces.begin() + index;
            StructurePiece* piece = *it;
            roadPieces.erase(it);
            piece->addChildren(startPiece, &mPieces, random);
        }
    }

    calculateBoundingBox();
    int nonRoadCount = 0;
    for (std::vector<StructurePiece*>::iterator pieceIt = mPieces.begin(); pieceIt != mPieces.end(); ++pieceIt)
    {
        if (dynamic_cast<VillagePieces::VillageRoadPiece*>(*pieceIt) == NULL)
            ++nonRoadCount;
    }
    mValid = nonRoadCount > 2;
}

VillageFeature::VillageStart::VillageStart() : StructureStart(), mValid(false), mDistance(0)
{
}

StructureStart* VillageFeature::VillageStart::Create()
{
    return new VillageStart();
}

bool VillageFeature::VillageStart::isValid()
{
    if (mBoundingBox->x0 < -mDistance / 2 || mBoundingBox->x1 > mDistance / 2 || mBoundingBox->z0 < -mDistance / 2
        || mBoundingBox->z1 > mDistance / 2)
    {
        mValid = false;
    }
    return mValid;
}

void VillageFeature::VillageStart::addAdditionalSaveData(DataOutputStream* output)
{
    StructureStart::addAdditionalSaveData(output);
    output->writeBoolean(mValid);
}

void VillageFeature::VillageStart::readAdditionalSaveData(DataInputStream* input)
{
    StructureStart::readAdditionalSaveData(input);
    mValid = input->readBoolean();
}

EStructureStart VillageFeature::VillageStart::GetType()
{
    return eStructureStart_Village;
}
