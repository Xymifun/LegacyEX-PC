#include "game/world/generation/StructurePlaceSettings.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/core/BoundingBox.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/core/System.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"

StructurePlaceSettings::StructurePlaceSettings()
{
    mMirror = Mirror::NONE;
    mRotation = Rotation::NONE;
    mIgnoreEntities = false;
    mIgnoreBlock = NULL;
    mChunkPos = NULL;
    mBoundingBox = NULL;
    mIgnoreStructureBlocks = true;
    mIntegrity = 1.0f;
    mRandom = NULL;
    mSeed = -1LL;
    mPlaceWaterBelowSeaLevel = false;
}

StructurePlaceSettings::~StructurePlaceSettings()
{
    delete mBoundingBox;
    delete mChunkPos;
}

void StructurePlaceSettings::read(DataInputStream* input)
{
    if (StructureFeatureIO::getVersion() < 3)
        return;

    mMirror = Mirror::VALUES[input->readInt()];
    mRotation = Rotation::VALUES[input->readInt()];
    mIgnoreEntities = input->readBoolean();
    mIgnoreStructureBlocks = input->readBoolean();
    mIntegrity = input->readFloat();
    mSeed = input->readLong();

    if (input->readBoolean())
    {
        if (mBoundingBox == NULL)
            mBoundingBox = new BoundingBox();
        mBoundingBox->read(input);
    }

    if (StructureFeatureIO::getVersion() < 4)
        return;

    mPlaceWaterBelowSeaLevel = input->readBoolean();
    int count = input->readInt();
    for (int i = 0; i < count; ++i)
    {
        int blockId = input->readInt();
        int auxValue = input->readInt();
        mSwapAuxValues[blockId] = auxValue;
    }
}

void StructurePlaceSettings::write(DataOutputStream* output)
{
    output->writeInt(mMirror->getType());
    output->writeInt(mRotation->getType());
    output->writeBoolean(mIgnoreEntities);
    output->writeBoolean(mIgnoreStructureBlocks);
    output->writeFloat(mIntegrity);
    output->writeLong(mSeed);
    output->writeBoolean(mBoundingBox != NULL);
    if (mBoundingBox != NULL)
        mBoundingBox->write(output);
    output->writeBoolean(mPlaceWaterBelowSeaLevel);
    output->writeInt(static_cast<int>(mSwapAuxValues.size()));
    boost::unordered_map<int, int>::const_iterator it = mSwapAuxValues.begin();
    for (; it != mSwapAuxValues.end(); ++it)
    {
        output->writeInt(it->first);
        output->writeInt(it->second);
    }
}

StructurePlaceSettings* StructurePlaceSettings::copy()
{
    StructurePlaceSettings* settings = new StructurePlaceSettings();
    settings->mMirror = mMirror;
    settings->mRotation = mRotation;
    settings->mIgnoreEntities = mIgnoreEntities;
    settings->mIgnoreBlock = mIgnoreBlock;
    settings->mChunkPos = mChunkPos;
    settings->mBoundingBox = mBoundingBox;
    settings->mIgnoreStructureBlocks = mIgnoreStructureBlocks;
    settings->mIntegrity = mIntegrity;
    settings->mRandom = mRandom;
    settings->mSeed = mSeed;
    settings->mPlaceWaterBelowSeaLevel = mPlaceWaterBelowSeaLevel;
    settings->mSwapAuxValues = mSwapAuxValues;
    return settings;
}

StructurePlaceSettings* StructurePlaceSettings::setMirror(Mirror* mirror)
{
    mMirror = mirror;
    return this;
}
StructurePlaceSettings* StructurePlaceSettings::setRotation(Rotation* rotation)
{
    mRotation = rotation;
    return this;
}
StructurePlaceSettings* StructurePlaceSettings::setIgnoreEntities(bool ignoreEntities)
{
    mIgnoreEntities = ignoreEntities;
    return this;
}
StructurePlaceSettings* StructurePlaceSettings::setIgnoreBlock(Block* block)
{
    mIgnoreBlock = block;
    return this;
}

StructurePlaceSettings* StructurePlaceSettings::setBoundingBox(BoundingBox* box)
{
    delete mBoundingBox;
    mBoundingBox = new BoundingBox(box);
    return this;
}

StructurePlaceSettings* StructurePlaceSettings::setRandom(Random* random)
{
    mRandom = random;
    return this;
}
StructurePlaceSettings* StructurePlaceSettings::setIntegrity(float integrity)
{
    mIntegrity = integrity;
    return this;
}

Mirror* StructurePlaceSettings::getMirror() const
{
    return mMirror;
}
Rotation* StructurePlaceSettings::getRotation() const
{
    return mRotation;
}

Random* StructurePlaceSettings::getRandom(const BlockPos* position, bool& generated)
{
    if (mRandom != NULL)
    {
        generated = false;
        return mRandom;
    }

    generated = true;
    if (mSeed >= 0)
    {
        return mSeed == 0 ? new Random(System::processTimeInMilliSecs()) : new Random(mSeed);
    }
    if (position == NULL)
    {
        return new Random(System::processTimeInMilliSecs());
    }
    int x = position->x;
    int z = position->z;
    return new Random(static_cast<int>(x * x * 4987142 + x * 5947611 + z * z * 4392871 + z * 389711 ^ 987234911));
}

float StructurePlaceSettings::getIntegrity()
{
    return mIntegrity;
}
bool StructurePlaceSettings::isIgnoreEntities()
{
    return mIgnoreEntities;
}
Block* StructurePlaceSettings::getIgnoreBlock()
{
    return mIgnoreBlock;
}

BoundingBox* StructurePlaceSettings::calculateBoundingBox(ChunkPos* chunkPos)
{
    BoundingBox* box = mBoundingBox;
    if (chunkPos != NULL)
    {
        delete mBoundingBox;
        box = new BoundingBox(chunkPos->x * 16, 0, chunkPos->z * 16, chunkPos->x * 16 + 15, 255, chunkPos->z * 16 + 15);
    }
    return box;
}

void StructurePlaceSettings::updateBoundingBoxFromChunkPos()
{
    mBoundingBox = calculateBoundingBox(mChunkPos);
}

BoundingBox* StructurePlaceSettings::getBoundingBox()
{
    if (mBoundingBox == NULL && mChunkPos != NULL)
        updateBoundingBoxFromChunkPos();
    return mBoundingBox;
}

bool StructurePlaceSettings::isIgnoreStructureBlocks()
{
    return mIgnoreStructureBlocks;
}

void StructurePlaceSettings::addSwapAuxValue(int blockId, int auxValue)
{
    mSwapAuxValues[blockId] = auxValue;
}

const BlockState* StructurePlaceSettings::getSwappedBlock(const BlockState* state)
{
    Block* block = state->getBlock();
    int blockId = Block::getId(block);
    bool isStrippedLog = block == Blocks::stripped_log_oak || block == Blocks::stripped_log_birch
                         || block == Blocks::stripped_log_acacia || block == Blocks::stripped_log_dark_oak
                         || block == Blocks::stripped_log_jungle || block == Blocks::stripped_log_spruce;
    bool isStairs = blockId == BlockId::OAK_STAIRS || blockId == BlockId::STONE_STAIRS
                    || blockId == BlockId::BRICK_STAIRS || blockId == BlockId::STONE_BRICK_STAIRS
                    || blockId == BlockId::NETHER_BRICK_STAIRS || blockId == BlockId::SANDSTONE_STAIRS
                    || blockId == BlockId::SPRUCE_STAIRS || blockId == BlockId::BIRCH_STAIRS
                    || blockId == BlockId::JUNGLE_STAIRS || blockId == BlockId::QUARTZ_STAIRS
                    || blockId == BlockId::ACACIA_STAIRS || blockId == BlockId::DARK_OAK_STAIRS
                    || blockId == BlockId::RED_SANDSTONE_STAIRS || blockId == BlockId::PURPUR_STAIRS;
    bool isTrapdoor = blockId == BlockId::TRAPDOOR || blockId == BlockId::SPRUCE_TRAPDOOR
                      || blockId == BlockId::BIRCH_TRAPDOOR || blockId == BlockId::JUNGLE_TRAPDOOR
                      || blockId == BlockId::ACACIA_TRAPDOOR || blockId == BlockId::DARK_OAK_TRAPDOOR;

    if (blockId == BlockId::LOG || blockId == BlockId::LOG2)
    {
        int data = block->convertBlockStateToLegacyData(state);
        int key = data & 3;
        if (blockId == BlockId::LOG2)
            key += 4;
        if (mSwapAuxValues.find(key) == mSwapAuxValues.end())
            return state;
        int replacement = mSwapAuxValues.at(key);
        if (replacement < 4)
            state = Blocks::log->getBlockState((data & 0xc) | replacement);
        else
            state = Blocks::log2->getBlockState((data & 0xc) | (replacement - 4));
    }
    else if (isStairs || block->mIsDoor || block->mIsFence || block->mIsFenceGate || isTrapdoor || isStrippedLog)
    {
        Block* stairs = Blocks::oak_stairs;
        Block* door = Blocks::wooden_door;
        Block* fence = Blocks::fence;
        Block* trapdoor = Blocks::trapdoor;
        Block* fenceGate = Blocks::fence_gate;
        Block* strippedLog = Blocks::stripped_log_oak;

        int key = 0;
        if (blockId == BlockId::SPRUCE_STAIRS || blockId == BlockId::SPRUCE_FENCE_GATE
            || blockId == BlockId::SPRUCE_FENCE || blockId == BlockId::SPRUCE_DOOR
            || blockId == BlockId::SPRUCE_TRAPDOOR || blockId == BlockId::STRIPPED_LOG_SPRUCE)
            key = 1;
        else if (blockId == BlockId::BIRCH_STAIRS || blockId == BlockId::BIRCH_FENCE_GATE
                 || blockId == BlockId::BIRCH_FENCE || blockId == BlockId::BIRCH_DOOR
                 || blockId == BlockId::BIRCH_TRAPDOOR || blockId == BlockId::STRIPPED_LOG_BIRCH)
            key = 2;
        else if (blockId == BlockId::JUNGLE_STAIRS || blockId == BlockId::JUNGLE_FENCE_GATE
                 || blockId == BlockId::JUNGLE_FENCE || blockId == BlockId::JUNGLE_DOOR
                 || blockId == BlockId::JUNGLE_TRAPDOOR || blockId == BlockId::STRIPPED_LOG_JUNGLE)
            key = 3;
        else if (blockId == BlockId::ACACIA_STAIRS || blockId == BlockId::ACACIA_FENCE_GATE
                 || blockId == BlockId::ACACIA_FENCE || blockId == BlockId::ACACIA_DOOR
                 || blockId == BlockId::ACACIA_TRAPDOOR || blockId == BlockId::STRIPPED_LOG_ACACIA)
            key = 4;
        else if (blockId == BlockId::DARK_OAK_STAIRS || blockId == BlockId::DARK_OAK_FENCE_GATE
                 || blockId == BlockId::DARK_OAK_FENCE || blockId == BlockId::DARK_OAK_DOOR
                 || blockId == BlockId::DARK_OAK_TRAPDOOR || blockId == BlockId::STRIPPED_LOG_DARK_OAK)
            key = 5;

        if (mSwapAuxValues.find(key) == mSwapAuxValues.end())
            return state;
        int replacement = mSwapAuxValues.at(key);
        int data = block->convertBlockStateToLegacyData(state);
        switch (replacement)
        {
        case 1:
            door = Blocks::spruce_door;
            fence = Blocks::spruce_fence;
            fenceGate = Blocks::spruce_fence_gate;
            stairs = Blocks::spruce_stairs;
            trapdoor = Blocks::spruce_trapdoor;
            strippedLog = Blocks::stripped_log_spruce;
            break;
        case 2:
            door = Blocks::birch_door;
            fence = Blocks::birch_fence;
            fenceGate = Blocks::birch_fence_gate;
            stairs = Blocks::birch_stairs;
            trapdoor = Blocks::birch_trapdoor;
            strippedLog = Blocks::stripped_log_birch;
            break;
        case 3:
            door = Blocks::jungle_door;
            fence = Blocks::jungle_fence;
            fenceGate = Blocks::jungle_fence_gate;
            stairs = Blocks::jungle_stairs;
            trapdoor = Blocks::jungle_trapdoor;
            strippedLog = Blocks::stripped_log_jungle;
            break;
        case 4:
            door = Blocks::acacia_door;
            fence = Blocks::acacia_fence;
            fenceGate = Blocks::acacia_fence_gate;
            stairs = Blocks::acacia_stairs;
            trapdoor = Blocks::acacia_trapdoor;
            strippedLog = Blocks::stripped_log_acacia;
            break;
        case 5:
            door = Blocks::dark_oak_door;
            fence = Blocks::dark_oak_fence;
            fenceGate = Blocks::dark_oak_fence_gate;
            stairs = Blocks::dark_oak_stairs;
            trapdoor = Blocks::dark_oak_trapdoor;
            strippedLog = Blocks::stripped_log_dark_oak;
            break;
        }

        if (isStairs)
            state = stairs->getBlockState(data);
        else if (block->mIsDoor)
            state = door->getBlockState(data);
        else if (block->mIsFence)
            state = fence->getBlockState(data);
        else if (block->mIsFenceGate)
            state = fenceGate->getBlockState(data);
        else if (isTrapdoor)
            state = trapdoor->getBlockState(data);
        else
        {
            if (isStrippedLog)
                block = strippedLog;
            state = block->getBlockState(data);
        }
    }
    else
    {
        if (block == Blocks::wooden_slab || block == Blocks::double_wooden_slab)
        {
            int data = block->convertBlockStateToLegacyData(state);
            int key = data & 7;
            if (mSwapAuxValues.find(key) != mSwapAuxValues.end())
            {
                int replacement = mSwapAuxValues.at(key);
                state = block->getBlockState((data & 8) | replacement);
            }
        }
        else if (block == Blocks::planks)
        {
            int key = block->convertBlockStateToLegacyData(state);
            if (mSwapAuxValues.find(key) != mSwapAuxValues.end())
            {
                int replacement = mSwapAuxValues.at(key);
                state = block->getBlockState(replacement);
            }
        }
    }
    return state;
}

void StructurePlaceSettings::placeWaterBelowSeaLevel(bool placeWater)
{
    mPlaceWaterBelowSeaLevel = placeWater;
}
bool StructurePlaceSettings::isPlacingWaterBelowSeaLevel() const
{
    return mPlaceWaterBelowSeaLevel;
}
