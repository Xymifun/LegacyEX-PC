#include "game/core/FillTool.hpp"

#include <math.h>
#include <algorithm>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Slot.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/InventoryMenu.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Math.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/world/block/BlockItem.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/Player.hpp"

FillTool::FillTool(ConsoleSchematicFile* schematic) : BlockPos(0, 0, 0), mPointB(0, 0, 0)
{
    mFillBlock = NULL;
    mReplaceOnlyDataMask = -1;
    mPointB.z = 3;
    mFillData = 0;
    mPointB.y = 3;
    mPointB.x = 3;
    mEdgeData = 0;
    mDimension = 0;
    mReplaceOnlyData = 0;
    mActive = false;
    mReplaceOnlyBlock = NULL;
    mReplaceOnly = false;
    mHasPointB = false;
    mField0x6C = false;
    mSchematic = schematic;
    mEdgeBlock = NULL;
    mOperation.mBounds = AABB::newPermanent(0.0, 0.0, 0.0, 3.0, 3.0, 3.0);
    mBrushStrength = 2.0f;
    mField0x50 = 0;
    mMode = 1;
    mFillDataMask = -1;
    mShape = 1;
    mHandleDataType = HANDLE_DATA_NONE;
    mLiquidType = 0;
    mSurfaceBrush = false;
}

FillTool::~FillTool()
{
    delete mOperation.mBounds;
}

void FillTool::Reset()
{
    mReplaceOnlyBlock = NULL;
    mReplaceOnlyData = 0;
    mReplaceOnlyDataMask = 0x00ffffff;
    mFillDataMask = -1;
    mHandleDataType = HANDLE_DATA_NONE;
}

int FillTool::IsWaterBlock(const BlockState* state)
{
    int result = 0;
    switch (Block::getId(state->getBlock()))
    {
    case BlockId::FLOWING_WATER:
    case BlockId::WATER:
        result = 1;
        break;
    }
    return result;
}

int FillTool::IsLavaBlock(const BlockState* state)
{
    int result = 0;
    switch (Block::getId(state->getBlock()))
    {
    case BlockId::FLOWING_LAVA:
    case BlockId::LAVA:
        result = 1;
        break;
    }
    return result;
}

int FillTool::IsValidSurfaceBlock(const BlockState* state)
{
    int result = 1;
    switch (Block::getId(state->getBlock()))
    {
    case BlockId::AIR:
    case BlockId::FLOWING_WATER:
    case BlockId::WATER:
    case BlockId::FLOWING_LAVA:
    case BlockId::LAVA:
    case BlockId::LOG:
    case BlockId::LEAVES:
    case BlockId::VINE:
    case BlockId::COCOA:
    case BlockId::LEAVES2:
    case BlockId::LOG2:
    case BlockId::KELP:
    case BlockId::CORAL_FAN:
    case BlockId::CORAL_FAN_DEAD:
    case BlockId::CORAL_FAN_HANG:
    case BlockId::CORAL_FAN_HANG2:
    case BlockId::CORAL_FAN_HANG3:
    case BlockId::SEA_GRASS:
    case BlockId::SEA_PICKLE:
    case BlockId::BUBBLE_COLUMN_SEGMENT:
        result = 0;
        break;
    }
    return result;
}

void FillTool::tick()
{
    if (!mActive)
        return;

    if (mOperation.mMode == 4)
    {
        int width = (int)(mOperation.mBounds->mMaxX - mOperation.mBounds->mMinX);
        int depth = (int)(mOperation.mBounds->mMaxZ - mOperation.mBounds->mMinZ);
        int baseZ = (int)mOperation.mBounds->mMinZ;
        int baseX = (int)mOperation.mBounds->mMinX;
        float halfWidth = (float)width * 0.5f;
        float halfDepth = (float)depth * 0.5f;
        unsigned int area = width * depth;
        float centreZ = ((float)mOperation.mBounds->mMinZ + halfDepth) - 0.5f;
        float radius = std::min(halfDepth, halfWidth);
        float centreX = ((float)mOperation.mBounds->mMinX + halfWidth) - 0.5f;

        int* surface = new int[area];
        for (unsigned int i = 0; i < area; ++i)
            surface[i] = 0;

        int index = 0;
        int top = -1;
        for (int z = 0; z < depth; ++z)
        {
            for (int x = 0; x < width; ++x)
            {
                int height = mOperation.mLevel->getHeightmapHeight(baseX + (int)x, baseZ + (int)z);
                surface[index] = height - 1;
                const BlockState* state = mOperation.mLevel->getBlockState(
                    BlockPos(baseX + (int)x, height - 1, baseZ + (int)z));
                if (state != NULL)
                {
                    if ((mLiquidType == 0 && IsWaterBlock(state)) || (mLiquidType == 1 && IsLavaBlock(state)))
                    {
                        if (top < surface[index])
                            top = surface[index];
                    }
                }
                ++index;
            }
        }

        if (top > 0)
        {
            const BlockState* liquid = Blocks::water->defaultBlockState();
            if (mLiquidType == 0)
                liquid = Blocks::water->defaultBlockState();
            else if (mLiquidType == 1)
                liquid = Blocks::lava->defaultBlockState();

            int row = 0;
            for (int z = 0; z < depth; ++z)
            {
                for (int x = 0; x < width; ++x)
                {
                    float worldX = (float)x + (float)mOperation.mBounds->mMinX;
                    float worldZ = (float)z + (float)mOperation.mBounds->mMinZ;
                    switch (mOperation.mShape)
                    {
                    case 1:
                    case 2:
                        if (sqrtf((worldX - centreX) * (worldX - centreX) + (worldZ - centreZ) * (worldZ - centreZ))
                            >= radius)
                            continue;
                        break;
                    }
                    MutableBlockPos pos((int)worldX, top, (int)worldZ);
                    const BlockState* state = mOperation.mLevel->getBlockState(pos);
                    while (surface[row + x] < pos.y && (state == NULL || state->getBlock() == Blocks::air))
                    {
                        mOperation.mLevel->setBlock(pos, liquid, Block::UPDATE_CLIENTS, false);
                        pos = MutableBlockPos(pos.below());
                        state = mOperation.mLevel->getBlockState(pos);
                    }
                }
                row += width;
            }
        }
    }
    else if (mOperation.mMode == 3)
    {
        int width = (int)(mOperation.mBounds->mMaxX - mOperation.mBounds->mMinX);
        int depth = (int)(mOperation.mBounds->mMaxZ - mOperation.mBounds->mMinZ);
        float halfWidth = (float)width * 0.5f;
        float halfDepth = (float)depth * 0.5f;
        float radius = std::min(halfDepth, halfWidth);
        float centreZ = ((float)mOperation.mBounds->mMinZ + halfDepth) - 0.5f;
        float centreX = ((float)mOperation.mBounds->mMinX + halfWidth) - 0.5f;

        for (int z = 0; z < depth; ++z)
        {
            for (int x = 0; x < width; ++x)
            {
                float worldX = (float)x + (float)mOperation.mBounds->mMinX;
                float worldZ = (float)z + (float)mOperation.mBounds->mMinZ;
                switch (mOperation.mShape)
                {
                case 1:
                case 2:
                    if (sqrtf((worldX - centreX) * (worldX - centreX) + (worldZ - centreZ) * (worldZ - centreZ))
                        >= radius)
                        continue;
                    break;
                }
                MutableBlockPos pos(mOperation.mLevel->getHeightmapPos(BlockPos((int)worldX, 0, (int)worldZ)).below());
                while (pos.y > 0 && !IsValidSurfaceBlock(mOperation.mLevel->getBlockState(pos)))
                    pos = MutableBlockPos(pos.below());
                std::vector<std::pair<const BlockState*, int> >::const_iterator layer = mOperation.mBrushBlocks.begin();
                while (layer != mOperation.mBrushBlocks.end() && pos.y > 0)
                {
                    const BlockState* brush = layer->first;
                    int thickness = layer->second;
                    for (int i = 0; i < thickness && pos.y > 0; ++i)
                    {
                        const BlockState* state = mOperation.mLevel->getBlockState(pos);
                        if (IsValidSurfaceBlock(state))
                            mOperation.mLevel->setBlock(pos, brush, Block::UPDATE_CLIENTS, false);
                        pos = MutableBlockPos(pos.below());
                    }
                    ++layer;
                }
            }
        }
    }
    else if (mOperation.mMode == 2)
    {
        int width = (int)(mOperation.mBounds->mMaxX - mOperation.mBounds->mMinX);
        float strength = mBrushStrength;
        int depth = (int)(mOperation.mBounds->mMaxZ - mOperation.mBounds->mMinZ);
        int radius = (int)ceil(strength * 2.57);
        int baseX = (int)mOperation.mBounds->mMinX;
        int baseZ = (int)mOperation.mBounds->mMinZ;
        int stride = width + radius * 2 + 1;
        unsigned int cells = (unsigned int)(stride * (depth + radius * 2 + 1));

        int* heights = new int[cells];
        for (unsigned int i = 0; i < cells; ++i)
            heights[i] = 0;

        int cursor = 0;
        for (int sampleZ = -radius; sampleZ < depth + radius + 1; ++sampleZ)
        {
            for (int sampleX = -radius; sampleX < width + radius + 1; ++sampleX)
            {
                MutableBlockPos pos(
                    mOperation.mLevel->getHeightmapPos(BlockPos(baseX + sampleX, 0, baseZ + sampleZ)).below());
                if (mOperation.mSkipSurfaceWater)
                {
                    while (pos.y > 0
                           && (IsWaterBlock(mOperation.mLevel->getBlockState(pos))
                               || IsWaterBlock(mOperation.mLevel->getExtraBlockState(pos))))
                        pos = MutableBlockPos(pos.below());
                }
                heights[cursor] = pos.y;
                ++cursor;
            }
        }

        int row = radius * stride + radius;
        for (int z = 0; z < depth; ++z)
        {
            for (int x = 0; x < width; ++x)
            {
                int centre = row + x;
                MutableBlockPos pos((int)mOperation.mBounds->mMinX + x, heights[centre],
                                    (int)mOperation.mBounds->mMinZ + z);
                float weightTotal = 0.0f;
                float heightTotal = 0.0f;
                for (int kernelZ = z - radius; kernelZ < z + radius + 1; ++kernelZ)
                {
                    int offsetZ = kernelZ - z;
                    for (int kernelX = x - radius; kernelX < x + radius + 1; ++kernelX)
                    {
                        int offsetX = kernelX - x;
                        float weight = 1.0f;
                        if (mOperation.mSmoothType == 0)
                        {
                            weight = expf((float)(-(offsetX * offsetX + offsetZ * offsetZ))
                                          / ((strength + strength) * strength))
                                     / ((6.2831855f * strength) * strength);
                        }
                        else if (mOperation.mSmoothType == 1)
                        {
                            weight = 1.0f;
                        }
                        weightTotal = weightTotal + weight;
                        heightTotal = (float)heights[(kernelZ + radius) * stride + (kernelX + radius)] * weight
                                      + heightTotal;
                    }
                }
                int target = Math::round(heightTotal / weightTotal);
                const BlockState* state = mOperation.mLevel->getBlockState(pos);
                while (target < pos.y)
                {
                    mOperation.mLevel->removeBlock(pos);
                    pos.setY(pos.y - 1);
                }
                mOperation.mLevel->setBlock(pos, state, Block::UPDATE_CLIENTS, false);
                pos.setY(pos.y - 1);
                while (heights[centre] < pos.y)
                {
                    mOperation.mLevel->setBlock(pos, state, Block::UPDATE_CLIENTS, false);
                    pos.setY(pos.y - 1);
                }
            }
            row += stride;
        }
    }
    else if (mOperation.mChunkX <= mOperation.mChunkMaxX && mOperation.mChunkZ <= mOperation.mChunkMaxZ)
    {
        LevelChunk* chunk = mOperation.mLevel->getChunk(mOperation.mChunkX, mOperation.mChunkZ);
        if (!chunk->isEmpty())
        {
            int startX = (int)std::max(floor(mOperation.mBounds->mMinX), chunk->x * 16.0);
            int endX = (int)std::min(floor(mOperation.mBounds->mMaxX), ((startX >> 4) << 4) + 16.0);

            int startY = (int)floor(mOperation.mBounds->mMinY);
            int endY = (int)floor(mOperation.mBounds->mMaxY);
            startY = std::max(startY, 0);
            startY = std::min(startY, 255);
            endY = std::max(endY, 0);
            endY = std::min(endY, 255);

            int startZ = std::max((int)floor(mOperation.mBounds->mMinZ), chunk->z << 4);
            int endZ = std::min((int)floor(mOperation.mBounds->mMaxZ), ((startZ >> 4) << 4) + 16);

            float halfHeight = (float)(endY - startY) * 0.5f;
            float halfDepth = (float)(mOperation.mBounds->mMaxZ - mOperation.mBounds->mMinZ) * 0.5f;
            float halfWidth = (float)(mOperation.mBounds->mMaxX - mOperation.mBounds->mMinX) * 0.5f;
            float radius = std::min(std::min(halfHeight, halfDepth), halfWidth);
            float centreX = ((float)mOperation.mBounds->mMinX + halfWidth) - 0.5f;
            float centreZ = ((float)mOperation.mBounds->mMinZ + halfDepth) - 0.5f;
            float centreY = ((float)startY + halfHeight) - 0.5f;

            int doubleSlabId = -1;
            if (mOperation.mReplaceOnlyBlock != NULL && HalfSlabBlock::isHalfSlab(mOperation.mReplaceOnlyBlock))
                doubleSlabId = Block::getId(HalfSlabBlock::GetDoubleSlabBlockFromSlab(mOperation.mReplaceOnlyBlock));

            MutableBlockPos pos(0, 0, 0);
            for (int y = startY; y < endY; ++y)
            {
                pos.setY(y);
                for (int x = startX; x < endX; ++x)
                {
                    pos.setX(x);
                    for (int z = startZ; z < endZ; ++z)
                    {
                        pos.setZ(z);
                        bool edge = false;
                        switch (mOperation.mShape)
                        {
                        default:
                            if (y == startY || y == endY - 1 || x == startX || x == endX - 1 || z == startZ
                                || z == endZ - 1)
                                edge = true;
                            break;
                        case 2:
                        {
                            float deltaX = (float)x - centreX;
                            float deltaY = (float)y - centreY;
                            float deltaZ = (float)z - centreZ;
                            float distance = (deltaX * deltaX) / (halfWidth * halfWidth)
                                             + (deltaY * deltaY) / (halfHeight * halfHeight)
                                             + (deltaZ * deltaZ) / (halfDepth * halfDepth);
                            if (distance >= 1.0f)
                            {
                                if (distance > 1.0f)
                                    continue;
                                edge = true;
                            }
                        }
                        break;
                        case 1:
                        {
                            float extent = radius * radius;
                            float deltaX = (float)x - centreX;
                            float deltaY = (float)y - centreY;
                            float deltaZ = (float)z - centreZ;
                            float distance = (deltaX * deltaX) / extent + (deltaY * deltaY) / extent
                                             + (deltaZ * deltaZ) / extent;
                            if (distance >= 1.0f)
                            {
                                if (distance > 1.0f)
                                    continue;
                                edge = true;
                            }
                        }
                        break;
                        }

                        const BlockState* state = mOperation.mLevel->getBlockState(BlockPos(x, y, z));
                        Block* block = state->getBlock();
                        int id = Block::getId(block);
                        int data = block->convertBlockStateToLegacyData(state);
                        if ((mOperation.mIgnoreAir && id == 0) || (mOperation.mIgnoreWater && (id == 9 || id == 8)))
                            continue;
                        int mask = mOperation.mFillDataMask;
                        switch (mOperation.mHandleDataType)
                        {
                        case 1:
                            if (mOperation.mReplaceOnlyBlock != NULL && id == mOperation.mReplaceOnlyBlockId)
                            {
                                mOperation.mLevel->setExtraBlock(pos, Blocks::air->defaultBlockState(),
                                                                 Block::UPDATE_CLIENTS, false);
                                mOperation.mLevel->setBlock(pos, mOperation.mFillBlock->getBlockState(data & mask),
                                                            Block::UPDATE_CLIENTS, false);
                            }
                            continue;
                        case 2:
                            if (mOperation.mReplaceOnlyBlock != NULL
                                && (HalfSlabBlock::isHalfSlab(block) || HalfSlabBlock::isFullSlab(block))
                                && (mask & (mOperation.mReplaceOnlyData ^ data)) == 0)
                            {
                                if (id == mOperation.mReplaceOnlyBlockId)
                                {
                                    int slabData = (data & ~mask) | (mOperation.mFillData & mask);
                                    mOperation.mLevel->setExtraBlock(pos, Blocks::air->defaultBlockState(),
                                                                     Block::UPDATE_CLIENTS, false);
                                    mOperation.mLevel->setBlock(pos, mOperation.mFillBlock->getBlockState(slabData),
                                                                Block::UPDATE_CLIENTS, false);
                                }
                                else if (id == doubleSlabId)
                                {
                                    Block* doubleSlab = HalfSlabBlock::GetDoubleSlabBlockFromSlab(
                                        mOperation.mFillBlock);
                                    if (doubleSlab != NULL)
                                    {
                                        int slabData = (data & ~mask) | (mOperation.mFillData & mask);
                                        mOperation.mLevel->setExtraBlock(pos, Blocks::air->defaultBlockState(),
                                                                         Block::UPDATE_CLIENTS, false);
                                        mOperation.mLevel->setBlock(pos, doubleSlab->getBlockState(slabData),
                                                                    Block::UPDATE_CLIENTS, false);
                                    }
                                }
                            }
                            continue;
                        }
                        if (mOperation.mReplaceOnlyBlock != NULL
                            && (id != mOperation.mReplaceOnlyBlockId
                                || (mOperation.mReplaceOnlyDataMask
                                    & (mOperation.mLevel->getBlockData(x, y, z) ^ mOperation.mReplaceOnlyData))
                                       != 0))
                            continue;
                        if (edge)
                        {
                            mOperation.mLevel->setExtraBlock(pos, Blocks::air->defaultBlockState(),
                                                             Block::UPDATE_CLIENTS, false);
                            mOperation.mLevel->setBlock(
                                pos, mOperation.mEdgeBlock->getBlockState(mOperation.mEdgeData & mask),
                                Block::UPDATE_CLIENTS, false);
                        }
                        else
                        {
                            mOperation.mLevel->setExtraBlock(pos, Blocks::air->defaultBlockState(),
                                                             Block::UPDATE_CLIENTS, false);
                            mOperation.mLevel->setBlock(
                                pos, mOperation.mFillBlock->getBlockState(mOperation.mFillData & mask),
                                Block::UPDATE_CLIENTS, false);
                        }
                    }
                }
            }
        }

        int nextZ = mOperation.mChunkZ + 1;
        if (nextZ <= mOperation.mChunkMaxZ)
        {
            mOperation.mChunkZ = nextZ;
        }
        else
        {
            mOperation.mChunkX = mOperation.mChunkX + 1;
            mOperation.mChunkZ = mOperation.mChunkMinZ;
        }
        return;
    }

    mActive = false;
}

void FillTool::PopulateSurfaceBrushBlock(boost::shared_ptr<Player> player)
{
    mSurfaceBlocks.clear();
    boost::shared_ptr<Inventory> inventory = player->mInventory;
    for (int i = 0; i < 9; ++i)
    {
        Slot* slot = player->mInventoryMenu->getSlot(InventoryMenu::USE_ROW_SLOT_START + i);
        not_null_ptr<ItemInstance> item = slot->getItem();
        if (item->isEmpty())
            break;
        BlockItem* blockItem = dynamic_cast<BlockItem*>(item->getItem());
        if (blockItem == NULL)
            break;
        Block* block = blockItem->getBlock();
        int data = blockItem->getLevelDataForAuxValue(item->getAuxValue());
        const BlockState* state = block->getBlockState(data);
        mSurfaceBlocks.push_back(std::make_pair(state, item->getCount()));
    }
}

void FillTool::Serialise(DataOutputStream* output)
{
    output->writeBlockPos(*this);
    output->writeBlockPos(mPointB);
    output->writeInt(Block::getId(mEdgeBlock));
    output->writeInt(mEdgeData);
    output->writeInt(Block::getId(mFillBlock));
    output->writeInt(mFillData);
    if (mReplaceOnlyBlock != NULL)
    {
        output->writeBoolean(true);
        output->writeInt(Block::getId(mReplaceOnlyBlock));
        output->writeInt(mReplaceOnlyData);
        output->writeInt(mReplaceOnlyDataMask);
    }
    else
    {
        output->writeBoolean(false);
    }
    output->writeInt(mFillDataMask);
    output->writeInt(mDimension);
    output->writeBoolean(mReplaceOnly);
    output->writeBoolean(mHasPointB);
    output->writeInt(mHandleDataType);
    output->writeInt(mShape);
    output->writeInt(mMode);
    output->writeInt(mLiquidType);
    output->writeInt(mField0x50);
    output->writeFloat(mBrushStrength);
    output->writeBoolean(mSurfaceBrush);
    if (mMode == 3)
    {
        output->writeInt((int)mSurfaceBlocks.size());
        std::vector<std::pair<const BlockState*, int> >::const_iterator entry;
        for (entry = mSurfaceBlocks.begin(); entry != mSurfaceBlocks.end(); ++entry)
        {
            output->writeUnsignedShort((unsigned short)Block::getElementId(entry->first));
            output->writeInt(entry->second);
        }
    }
}

void FillTool::Deserialise(DataInputStream* input)
{
    Reset();
    *static_cast<BlockPos*>(this) = MutableBlockPos(input->readBlockPos());
    mPointB = MutableBlockPos(input->readBlockPos());
    mEdgeBlock = Block::byId(input->readInt());
    mEdgeData = input->readInt();
    mFillBlock = Block::byId(input->readInt());
    mFillData = input->readInt();
    if (input->readBoolean())
    {
        mReplaceOnlyBlock = Block::byId(input->readInt());
        mReplaceOnlyData = input->readInt();
        mReplaceOnlyDataMask = input->readInt();
    }
    mFillDataMask = input->readInt();
    mDimension = input->readInt();
    mReplaceOnly = input->readBoolean();
    mHasPointB = input->readBoolean();
    mHandleDataType = input->readInt();
    mShape = input->readInt();
    mMode = input->readInt();
    mLiquidType = input->readInt();
    mField0x50 = input->readInt();
    mBrushStrength = input->readFloat();
    mSurfaceBrush = input->readBoolean();
    if (mMode == 3)
    {
        mSurfaceBlocks.clear();
        int count = input->readInt();
        for (int i = 0; i < count; ++i)
        {
            int elementId = input->readUnsignedShort();
            int thickness = input->readInt();
            mSurfaceBlocks.push_back(std::make_pair(Block::getStateByElementId(elementId), thickness));
        }
    }
    else
    {
        mSurfaceBlocks.clear();
    }
}
