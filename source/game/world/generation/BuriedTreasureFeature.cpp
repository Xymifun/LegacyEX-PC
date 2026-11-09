#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/world/generation/BuriedTreasureFeature.hpp"
#include "game/core/EntityType.hpp"

#include <vector>

#include "game/core/BuiltInLootTables.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Rotation.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/biome/FjFeatureBiomeRequirements.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/generation/StructurePiece.hpp"

static std::vector<Biome*> gBuriedTreasureSpawnBiomes;
static std::vector<Biome*> gBuriedTreasureAllowedBiomes;
static FjFeatureBiomeRequirements gBuriedTreasureBiomeRequirements;

static const int BURIED_TREASURE_SALT = 0x100fe9d;
static const int BURIED_TREASURE_TERRAIN_FEATURE_ID = 10;

void BuriedTreasureFeature::staticCtor()
{
    gBuriedTreasureSpawnBiomes.push_back(Biome::BEACH);
    gBuriedTreasureSpawnBiomes.push_back(Biome::COLD_BEACH);
    gBuriedTreasureSpawnBiomes.push_back(Biome::STONE_BEACH);
    gBuriedTreasureSpawnBiomes.push_back(Biome::MUSHROOM_ISLAND_SHORE);

    for (int first = 0; first < 256; first += 8)
    {
        for (int offset = 0; offset < 8; ++offset)
        {
            Biome* biome = Biome::byId(first + offset);
            if (biome != NULL && biome->getScale() < 0.4f)
                gBuriedTreasureAllowedBiomes.push_back(biome);
        }
    }

    gBuriedTreasureBiomeRequirements = FjFeatureBiomeRequirements(&gBuriedTreasureSpawnBiomes, 0,
                                                                  &gBuriedTreasureAllowedBiomes, 16);
}

BuriedTreasureFeature::BuriedTreasureFeature(int) : StructureFeature()
{
    mFeatureChunk = FjIsFeatureChunk(&gBuriedTreasureBiomeRequirements, 40, 4, 2);
    mReducedFeatureChunk = FjIsFeatureChunk::CreateReduced(mFeatureChunk);
}

std::wstring BuriedTreasureFeature::getFeatureName()
{
    return L"Buried Treasure";
}

BlockPos* BuriedTreasureFeature::getNearestGeneratedFeature(Level* levelIn, const BlockPos& pos,
                                                            bool skipExistingChunks)
{
    level = levelIn;
    return StructureFeature::findNearestFeaturePositionBySpacing(levelIn, this, pos, 4, 2, BURIED_TREASURE_SALT, true,
                                                                 100, skipExistingChunks);
}

StructureStart* BuriedTreasureFeature::createStructureStart(int chunkX, int chunkZ)
{
    return new BuriedTreasureStart(this, level, random, chunkX, chunkZ);
}

bool BuriedTreasureFeature::isFeatureChunk(int chunkX, int chunkZ, bool flatWorld)
{
    int sizeInChunks = level->getLevelData()->getXZSize();
    FjIsFeatureChunk* featureChunk = FjIsFeatureChunk::UseReducedSpacing(sizeInChunks, flatWorld)
                                         ? &mReducedFeatureChunk
                                         : &mFeatureChunk;
    ChunkPos center = featureChunk->getCenterChunk(chunkX, chunkZ);
    bool result = featureChunk->isFeatureChunk(*level->getRandomFor(center.x, center.z, BURIED_TREASURE_SALT),
                                               level->getBiomeSource(), sizeInChunks, chunkX, chunkZ);
    result = canPlaceFeatureInChunk(result, eFeatureType_BuriedTreasure, chunkX, chunkZ);
    if (result)
        gApp.AddTerrainFeaturePosition(static_cast<_eTerrainFeatureType>(BURIED_TREASURE_TERRAIN_FEATURE_ID), chunkX,
                                       chunkZ);
    return result;
}

BuriedTreasureFeature::BuriedTreasurePiece::BuriedTreasurePiece(int x, int z) : TemplateStructurePiece(4)
{
    setOrientation(Direction::NORTH);
    mBoundingBox = new BoundingBox(x, 64, z, x + 1, 73, z + 1);
}

bool BuriedTreasureFeature::BuriedTreasurePiece::postProcess(Level* levelIn, Random& randomIn, BoundingBox* box)
{
    BlockPos origin(getWorldX(0, 0), getWorldY(1), getWorldZ(0, 0));
    if (!box->isInside(origin))
        return false;

    origin = BlockPos(origin.x, levelIn->getTopSolidBlockPos(origin).y, origin.z);
    origin = origin.offset(0, -(randomIn.nextInt(3) + 3), 0);
    while (levelIn->getBlock(origin.below()) == Blocks::air)
        origin = origin.below();

    levelIn->setBlock(origin, Blocks::chest->defaultBlockState(), Block::UPDATE_ALL, false);
    boost::shared_ptr<BlockEntity> blockEntity = levelIn->getBlockEntity(origin);
    if (blockEntity != NULL && eTYPE_DERIVED_FROM(BlockEntityType::CHEST, blockEntity->GetType()))
    {
        boost::shared_ptr<ChestBlockEntity> chest = boost::static_pointer_cast<ChestBlockEntity>(blockEntity);
        chest->setLootTable(&BuiltInLootTables::BURIEDTREASURE, static_cast<long long>(randomIn.nextInt()));
    }
    return true;
}

void BuriedTreasureFeature::BuriedTreasurePiece::handleDataMarker(const std::wstring& marker, const BlockPos& position,
                                                                  Level* levelIn, Random& randomIn, BoundingBox* box)
{
    if (marker.find(L"Chest") != 0)
        return;

    Rotation* rotation = mPlaceSettings->getRotation();
    const BlockState* state = Blocks::chest->defaultBlockState();
    if (marker.compare(0, marker.size(), L"ChestWest") == 0)
    {
        {
            TypedBoxed<const Direction*> facing(rotation->rotate(Direction::WEST));
            state = state->setBoxedValue(static_cast<const Property*>(ChestBlock::FACING), &facing);
        }
        createChest(levelIn, box, randomIn, position, &BuiltInLootTables::WOODLAND_MANSION, state);
    }
    else if (marker.compare(0, marker.size(), L"ChestEast") == 0)
    {
        {
            TypedBoxed<const Direction*> facing(rotation->rotate(Direction::EAST));
            state = state->setBoxedValue(static_cast<const Property*>(ChestBlock::FACING), &facing);
        }
        createChest(levelIn, box, randomIn, position, &BuiltInLootTables::WOODLAND_MANSION, state);
    }
    else if (marker.compare(0, marker.size(), L"ChestSouth") == 0)
    {
        {
            TypedBoxed<const Direction*> facing(rotation->rotate(Direction::SOUTH));
            state = state->setBoxedValue(static_cast<const Property*>(ChestBlock::FACING), &facing);
        }
        createChest(levelIn, box, randomIn, position, &BuiltInLootTables::WOODLAND_MANSION, state);
    }
    else
    {
        if (marker.compare(0, marker.size(), L"ChestNorth") == 0)
        {
            {
                TypedBoxed<const Direction*> facing(rotation->rotate(Direction::NORTH));
                state = state->setBoxedValue(static_cast<const Property*>(ChestBlock::FACING), &facing);
            }
        }
        createChest(levelIn, box, randomIn, position, &BuiltInLootTables::WOODLAND_MANSION, state);
    }
}

EStructurePiece BuriedTreasureFeature::BuriedTreasurePiece::GetType()
{
    return eStructurePiece_BuriedTreasurePiece;
}

BuriedTreasureFeature::BuriedTreasureStart::BuriedTreasureStart(BuriedTreasureFeature* feature, Level* levelIn, Random&,
                                                                int chunkX, int chunkZ)
    : StructureStart(chunkX, chunkZ),
      mFeature(feature)
{
    int z = chunkZ * 16;
    int x = chunkX * 16;
    if (levelIn->getBiome(BlockPos(x + 8, 0, z + 8)) != NULL)
        mPieces.push_back(new BuriedTreasurePiece(x + 8, z + 8));
    calculateBoundingBox();
}

BuriedTreasureFeature::BuriedTreasureStart::BuriedTreasureStart() : StructureStart(0, 0)
{
}

EStructureStart BuriedTreasureFeature::BuriedTreasureStart::GetType()
{
    return eStructureStart_BuriedTreasure;
}

StructureStart* BuriedTreasureFeature::BuriedTreasureStart::Create()
{
    return new BuriedTreasureStart();
}
