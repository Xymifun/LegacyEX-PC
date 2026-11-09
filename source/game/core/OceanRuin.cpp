#include "game/core/OceanRuin.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/BoundingBox.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/StructurePiece.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"
#include "game/world/generation/StructureTemplate.hpp"

OceanRuin::OceanRuin()
    : ScatteredFeaturePieces::ScatteredFeaturePiece(),
      mBlockState(Blocks::water->defaultBlockState()),
      mIsLarge(false),
      mIsWarm(false)
{
}

OceanRuin::OceanRuin(Level* level, Random& random, int x, int z, int width, int height, int depth, bool isWarm,
                     bool isLarge)
    : ScatteredFeaturePieces::ScatteredFeaturePiece(level, random, x, 0, z, width, height, depth)
{
    setOrientation(Direction::SOUTH);
    setBoundingBox(new BoundingBox(x, 0, z, x + width - 1, height - 1, z + depth - 1));
    mBlockState = Blocks::water->defaultBlockState();
    mIsLarge = isLarge;
    mIsWarm = isWarm;
    setRotation(Rotation::getRandom(random));
}

eMinecraftStructureTemplate OceanRuin::getBigWarmRuin(Random& random)
{
    eMinecraftStructureTemplate ruins[4] = {
        eMinecraftStructureTemplate_OceanRuin_BigRuinWarm4, eMinecraftStructureTemplate_OceanRuin_BigRuinWarm5,
        eMinecraftStructureTemplate_OceanRuin_BigRuinWarm6, eMinecraftStructureTemplate_OceanRuin_BigRuinWarm7};
    return ruins[random.nextInt(4)];
}

eMinecraftStructureTemplate OceanRuin::getSmallWarmRuin(Random& random)
{
    eMinecraftStructureTemplate ruins[3] = {eMinecraftStructureTemplate_OceanRuin_RuinWarm1,
                                            eMinecraftStructureTemplate_OceanRuin_RuinWarm2,
                                            eMinecraftStructureTemplate_OceanRuin_RuinWarm3};
    return ruins[random.nextInt(3)];
}

eMinecraftStructureTemplate OceanRuin::getRuin(int index, bool large, ERuinType type)
{
    switch (type)
    {
    case eRuinType_Cold:
    {
        eMinecraftStructureTemplate bigRuins[4] = {
            eMinecraftStructureTemplate_OceanRuin_BigRuin1Brick, eMinecraftStructureTemplate_OceanRuin_BigRuin2Brick,
            eMinecraftStructureTemplate_OceanRuin_BigRuin3Brick, eMinecraftStructureTemplate_OceanRuin_BigRuin8Brick};
        eMinecraftStructureTemplate smallRuins[8] = {
            eMinecraftStructureTemplate_OceanRuin_Ruin1Brick, eMinecraftStructureTemplate_OceanRuin_Ruin2Brick,
            eMinecraftStructureTemplate_OceanRuin_Ruin3Brick, eMinecraftStructureTemplate_OceanRuin_Ruin4Brick,
            eMinecraftStructureTemplate_OceanRuin_Ruin5Brick, eMinecraftStructureTemplate_OceanRuin_Ruin6Brick,
            eMinecraftStructureTemplate_OceanRuin_Ruin7Brick, eMinecraftStructureTemplate_OceanRuin_Ruin8Brick};
        return large ? bigRuins[index] : smallRuins[index];
    }
    case eRuinType_Warm:
    {
        eMinecraftStructureTemplate bigRuins[4] = {eMinecraftStructureTemplate_OceanRuin_BigRuin1Cracked,
                                                   eMinecraftStructureTemplate_OceanRuin_BigRuin2Cracked,
                                                   eMinecraftStructureTemplate_OceanRuin_BigRuin3Cracked,
                                                   eMinecraftStructureTemplate_OceanRuin_BigRuin8Cracked};
        eMinecraftStructureTemplate smallRuins[8] = {
            eMinecraftStructureTemplate_OceanRuin_Ruin1Cracked, eMinecraftStructureTemplate_OceanRuin_Ruin2Cracked,
            eMinecraftStructureTemplate_OceanRuin_Ruin3Cracked, eMinecraftStructureTemplate_OceanRuin_Ruin4Cracked,
            eMinecraftStructureTemplate_OceanRuin_Ruin5Cracked, eMinecraftStructureTemplate_OceanRuin_Ruin6Cracked,
            eMinecraftStructureTemplate_OceanRuin_Ruin7Cracked, eMinecraftStructureTemplate_OceanRuin_Ruin8Cracked};
        return large ? bigRuins[index] : smallRuins[index];
    }
    case eRuinType_ColdBrick:
    {
        eMinecraftStructureTemplate bigRuins[4] = {
            eMinecraftStructureTemplate_OceanRuin_BigRuin1Mossy, eMinecraftStructureTemplate_OceanRuin_BigRuin2Mossy,
            eMinecraftStructureTemplate_OceanRuin_BigRuin3Mossy, eMinecraftStructureTemplate_OceanRuin_BigRuin8Mossy};
        eMinecraftStructureTemplate smallRuins[8] = {
            eMinecraftStructureTemplate_OceanRuin_Ruin1Mossy, eMinecraftStructureTemplate_OceanRuin_Ruin2Mossy,
            eMinecraftStructureTemplate_OceanRuin_Ruin3Mossy, eMinecraftStructureTemplate_OceanRuin_Ruin4Mossy,
            eMinecraftStructureTemplate_OceanRuin_Ruin5Mossy, eMinecraftStructureTemplate_OceanRuin_Ruin6Mossy,
            eMinecraftStructureTemplate_OceanRuin_Ruin7Mossy, eMinecraftStructureTemplate_OceanRuin_Ruin8Mossy};
        return large ? bigRuins[index] : smallRuins[index];
    }
    default:
        return eMinecraftStructureTemplate_Invalid;
    }
}

void OceanRuin::updateChestReplacer(eMinecraftStructureTemplate structureTemplate)
{
    if (mIsWarm)
    {
        mBlockState = Blocks::sandstone->defaultBlockState();
        return;
    }

    const BlockState* state = Blocks::water->defaultBlockState();
    if (structureTemplate == eMinecraftStructureTemplate_OceanRuin_Ruin3Brick)
    {
        mBlockState = Blocks::stonebrick->defaultBlockState();
        return;
    }
    if (structureTemplate == eMinecraftStructureTemplate_OceanRuin_BigRuin1Brick
        || structureTemplate == eMinecraftStructureTemplate_OceanRuin_BigRuin8Brick
        || structureTemplate == eMinecraftStructureTemplate_OceanRuin_BigRuin2Brick)
        state = Blocks::gravel->defaultBlockState();
    mBlockState = state;
}

const StructureTemplate* OceanRuin::placeRuin(Random& random, StructureManager* structureManager, Level* level,
                                              const BlockPos& pos, StructurePlaceSettings& settings)
{
    float integrity = mIsLarge ? 0.9f : 0.8f;
    if (mIsWarm)
    {
        if (mIsLarge)
        {
            eMinecraftStructureTemplate ruin = getBigWarmRuin(random);
            updateChestReplacer(ruin);
            const StructureTemplate* result = structureManager->getOrCreate(ruin);
            settings.setIntegrity(integrity);
            result->placeInWorld(level, pos, &settings);
            return result;
        }
        else
        {
            eMinecraftStructureTemplate ruin = getSmallWarmRuin(random);
            updateChestReplacer(ruin);
            const StructureTemplate* result = structureManager->getOrCreate(ruin);
            settings.setIntegrity(integrity);
            result->placeInWorld(level, pos, &settings);
            return result;
        }
    }

    int index = random.nextInt(mIsLarge ? 4 : 8);
    eMinecraftStructureTemplate base = getRuin(index, mIsLarge, eRuinType_Cold);
    eMinecraftStructureTemplate second = getRuin(index, mIsLarge, eRuinType_Warm);
    eMinecraftStructureTemplate third = getRuin(index, mIsLarge, eRuinType_ColdBrick);
    updateChestReplacer(base);
    const StructureTemplate* result = structureManager->getOrCreate(base);
    settings.setIntegrity(integrity);
    result->placeInWorld(level, pos, &settings);
    settings.setIntegrity(0.7f);
    structureManager->getOrCreate(second)->placeInWorld(level, pos, &settings);
    settings.setIntegrity(0.5f);
    structureManager->getOrCreate(third)->placeInWorld(level, pos, &settings);
    return result;
}

BlockPos OceanRuin::adaptPosition(const BlockPos& pos, Mirror* mirror, Rotation* rotation, Level* level)
{
    BlockPos end =
        StructureTemplate::transform(BlockPos(mWidth - 1, 0, mDepth - 1), mirror, rotation).offset(pos.x, pos.y, pos.z);
    int posY = pos.y;
    int top = posY - 1;
    int lowest = 512;
    int lowCount = 0;
    BlockPos min(end.x < pos.x ? end.x : pos.x, end.y < pos.y ? end.y : pos.y, end.z < pos.z ? end.z : pos.z);
    BlockPos max(pos.x < end.x ? end.x : pos.x, end.y < pos.y ? pos.y : end.y, pos.z < end.z ? end.z : pos.z);
    for (int x = min.x; x <= max.x; ++x)
    {
        for (int z = min.z; z <= max.z; ++z)
        {
            for (int y = min.y; y <= max.y; ++y)
            {
                int height = y - 1;
                while ((level->isEmptyBlock(BlockPos(x, height, z))
                        || level->getBlockState(BlockPos(x, height, z))->getMaterial()->isLiquid())
                       && height > 1)
                    --height;
                if (height < lowest)
                    lowest = height;
                if (height < top - 2)
                    ++lowCount;
            }
        }
    }
    if (top - lowest > 2 && lowCount > mWidth + 4)
        return BlockPos(pos.x, lowest + 3, pos.z);
    return pos;
}

void OceanRuin::fillBelow(const BlockPos& pos, Mirror* mirror, Rotation* rotation, Level* level)
{
    BlockPos end =
        StructureTemplate::transform(BlockPos(mWidth - 1, 0, mDepth - 1), mirror, rotation).offset(pos.x, pos.y, pos.z);
    int top = pos.y - 1;
    BlockPos min(end.x < pos.x ? end.x : pos.x, end.y < pos.y ? end.y : pos.y, end.z < pos.z ? end.z : pos.z);
    BlockPos max(pos.x < end.x ? end.x : pos.x, pos.y < end.y ? end.y : pos.y, pos.z < end.z ? end.z : pos.z);
    for (int x = min.x; x <= max.x; ++x)
    {
        for (int z = min.z; z <= max.z; ++z)
        {
            for (int y = min.y; y <= max.y; ++y)
            {
                int height = y - 1;
                while ((level->isEmptyBlock(BlockPos(x, height, z))
                        || level->getBlockState(BlockPos(x, height, z))->getMaterial()->isLiquid())
                       && height > 1)
                    --height;
                for (int h = height; h < top + 1; ++h)
                {
                    if (level->mRandom->rollPercentageChance(82))
                        level->setBlock(BlockPos(x, h, z), Blocks::gravel->defaultBlockState(), Block::UPDATE_CLIENTS,
                                        false);
                }
            }
        }
    }
}

bool OceanRuin::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (!updateAverageGroundHeight(level, box, -2, false))
        return true;
    BoundingBox* boundingBox = getBoundingBox();
    if (boundingBox == NULL)
        return true;
    if (!box->isInside(BlockPos(boundingBox->x0, boundingBox->y0, boundingBox->z0)))
        return true;

    BlockPos pos(getWorldX(0, 0), getWorldY(0), getWorldZ(0, 0));
    int offsetX = 0;
    int offsetZ = 0;
    switch (getRotation()->getType())
    {
    case Rotation::eTYPE_None:
        break;
    case Rotation::eTYPE_Clockwise90:
        offsetX = mWidth - 1;
        break;
    case Rotation::eTYPE_Clockwise180:
        offsetX = mWidth - 1;
        offsetZ = mDepth - 1;
        break;
    case Rotation::eTYPE_CounterClockwise90:
        offsetZ = mDepth - 1;
        break;
    }
    pos = pos.offset(offsetX, 0, offsetZ);

    StructureManager* structureManager = level->getLevelStorage()->getStructureManager();
    pos = adaptPosition(pos, Mirror::NONE, getRotation(), level);
    boundingBox->move(0, pos.y - boundingBox->y0, 0);

    StructurePlaceSettings settings;
    settings.setMirror(Mirror::NONE);
    settings.setRotation(getRotation());
    settings.setIgnoreBlock(Blocks::air);
    settings.setBoundingBox(boundingBox);
    fillBelow(pos, Mirror::NONE, getRotation(), level);
    const StructureTemplate* structure = placeRuin(random, structureManager, level, pos, settings);
    StructureTemplate::MarkerMap markers = structure->getMarkers(pos, &settings);
    StructureTemplate::MarkerMap::iterator it;
    for (it = markers.begin(); it != markers.end(); ++it)
    {
        BlockPos markerPos = it->first;
        std::wstring marker = it->second;
        if (marker != L"chest")
            continue;

        level->setBlock(markerPos, mBlockState, Block::UPDATE_ALL, false);
        level->setBlock(markerPos.below(),
                        Blocks::chest->defaultBlockState()->setValue(ChestBlock::FACING, Direction::EAST),
                        Block::UPDATE_ALL, false);
        level->setExtraBlock(markerPos.below(), Blocks::water->defaultBlockState(), Block::UPDATE_ALL, false);
        boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(markerPos.below());
        if (entity != NULL && eTYPE_DERIVED_FROM(BlockEntityType::CHEST, entity->GetType()))
        {
            boost::shared_ptr<ChestBlockEntity> chest = boost::static_pointer_cast<ChestBlockEntity>(entity);
            ResourceLocation* lootTable = mIsLarge ? &BuiltInLootTables::UNDERWATER_RUIN_BIG
                                                   : &BuiltInLootTables::UNDERWATER_RUIN_SMALL;
            chest->setLootTable(lootTable, random.nextLong());
        }
    }
    return true;
}

void OceanRuin::addAdditionalSaveData(DataOutputStream* output)
{
    output->writeInt(Block::getId(mBlockState->getBlock()));
    output->writeBoolean(mIsLarge);
    output->writeBoolean(mIsWarm);
}

void OceanRuin::readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager)
{
    mBlockState = Block::byId(input->readInt())->defaultBlockState();
    mIsLarge = input->readBoolean();
    mIsWarm = input->readBoolean();
}

EStructurePiece OceanRuin::GetType()
{
    return eStructurePiece_OceanRuin;
}

StructurePiece* OceanRuin::Create()
{
    return new OceanRuin();
}
