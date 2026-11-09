#include "game/world/generation/StructureTemplate.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <climits>

#include <boost/shared_ptr.hpp>

#include "game/core/BoundingBox.hpp"
#include "game/core/AnonymousDataWalker.hpp"
#include "game/core/DataWalker.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/DoubleTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Container.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/NbtUtils.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockRotProcessor.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"
#include "game/world/generation/StructureProcessor.hpp"

static const std::wstring PALETTE_TAG = L"palette";
static const std::wstring ENTITIES_TAG = L"entities";
static const std::wstring BLOCKS_TAG = L"blocks";
static const std::wstring BLOCK_POS_TAG = L"pos";
static const std::wstring BLOCK_STATE_TAG = L"state";
static const std::wstring BLOCK_NBT_TAG = L"nbt";
static const std::wstring ENTITY_POS_TAG = L"pos";
static const std::wstring ENTITY_BLOCK_POS_TAG = L"blockPos";
static const std::wstring ENTITY_NBT_TAG = L"nbt";
static const std::wstring SIZE_TAG = L"size";
static const std::wstring AUTHOR_TAG = L"author";

StructureTemplate::StructureBlockInfo::~StructureBlockInfo()
{
    delete mNbt;
}

StructureTemplate::StructureEntityInfo::~StructureEntityInfo()
{
    delete mPos;
    delete mNbt;
}

StructureTemplate::StructureTemplate()
{
    mSize = BlockPos::ZERO;
    mAuthor = L"?";
}

StructureTemplate::~StructureTemplate()
{
    for (std::vector<StructureBlockInfo*>::iterator block = mBlocks.begin(); block != mBlocks.end(); ++block)
    {
        delete *block;
    }

    for (std::vector<StructureEntityInfo*>::iterator entity = mEntities.begin(); entity != mEntities.end(); ++entity)
    {
        delete *entity;
    }
}

BlockPos StructureTemplate::getSize() const
{
    return mSize;
}

BlockPos StructureTemplate::transform(const BlockPos& pos, Mirror* mirror, Rotation* rotation)
{
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;

    bool mirrored = true;

    switch (mirror->getType())
    {
    case Mirror::eTYPE_LeftRight:
        z = -z;
        break;

    case Mirror::eTYPE_FrontBack:
        x = -x;
        break;

    default:
        mirrored = false;
        break;
    }

    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise90:
        return BlockPos(-z, y, x);

    case Rotation::eTYPE_Clockwise180:
        return BlockPos(-x, y, -z);

    case Rotation::eTYPE_CounterClockwise90:
        return BlockPos(z, y, -x);

    default:
        return mirrored ? BlockPos(x, y, z) : pos;
    }
}

BlockPos StructureTemplate::calculateRelativePosition(const StructurePlaceSettings* settings, const BlockPos& pos)
{
    return transform(pos, settings->getMirror(), settings->getRotation());
}

StructureTemplate::MarkerMap StructureTemplate::getMarkers(const BlockPos& pos, StructurePlaceSettings* settings) const
{
    MarkerMap markers;

    BoundingBox* boundingBox = settings->getBoundingBox();

    for (std::vector<StructureBlockInfo*>::const_iterator it = mBlocks.begin(); it != mBlocks.end(); ++it)
    {
        StructureBlockInfo* blockInfo = *it;

        BlockPos blockPos = calculateRelativePosition(settings, blockInfo->mPos).offset(pos);

        if (boundingBox != NULL && !boundingBox->isInside(blockPos))
        {
            continue;
        }

        if (blockInfo->mState->getBlock() == Blocks::structure_block && blockInfo->mNbt != NULL)
        {
            markers.insert(std::make_pair(blockPos, blockInfo->mNbt->getString(L"metadata")));
        }
    }

    return markers;
}

BlockPos StructureTemplate::calculateConnectedPosition(const StructurePlaceSettings* settings, const BlockPos& pos,
                                                       const StructurePlaceSettings* connectedSettings,
                                                       const BlockPos& connectedPos) const
{
    return calculateRelativePosition(settings, pos)
        .subtract(calculateRelativePosition(connectedSettings, connectedPos));
}

Vec3* StructureTemplate::transform(Vec3* pos, Mirror* mirror, Rotation* rotation)
{
    double x = pos->x;
    double y = pos->y;
    double z = pos->z;

    bool mirrored = true;

    switch (mirror->getType())
    {
    case Mirror::eTYPE_LeftRight:
        z = 1.0 - z;
        break;

    case Mirror::eTYPE_FrontBack:
        x = 1.0 - x;
        break;

    default:
        mirrored = false;
        break;
    }

    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise90:
        return Vec3::newTemp(1.0 - z, y, x);

    case Rotation::eTYPE_Clockwise180:
        return Vec3::newTemp(1.0 - x, y, 1.0 - z);

    case Rotation::eTYPE_CounterClockwise90:
        return Vec3::newTemp(z, y, 1.0 - x);

    default:
        return mirrored ? Vec3::newTemp(x, y, z) : pos;
    }
}

void StructureTemplate::placeEntities(Level* level, const BlockPos& pos, Mirror* mirror, Rotation* rotation,
                                      BoundingBox* boundingBox) const
{
    for (std::vector<StructureEntityInfo*>::const_iterator it = mEntities.begin(); it != mEntities.end(); it++)
    {
        StructureEntityInfo* entityInfo = *it;

        BlockPos blockPos = transform(entityInfo->mBlockPos, mirror, rotation).offset(pos);

        if (boundingBox != NULL && !boundingBox->isInside(blockPos))
        {
            continue;
        }

        CompoundTag* nbt = entityInfo->mNbt;

        Vec3* entityPos =
            transform(entityInfo->mPos, mirror, rotation)->add((double)pos.x, (double)pos.y, (double)pos.z);

        ListTag* posTag = new ListTag();
        posTag->add(new DoubleTag(entityPos->x));
        posTag->add(new DoubleTag(entityPos->y));
        posTag->add(new DoubleTag(entityPos->z));

        nbt->put(L"Pos", posTag);

        boost::shared_ptr<Entity> entity;
        entity = EntityIO::newEntity(nbt, level);

        if (entity != NULL)
        {
            float yRot = entity->mirror(mirror);
            yRot = yRot + (entity->mYRot - entity->rotate(rotation));

            entity->moveTo(entityPos->x, entityPos->y, entityPos->z, yRot, entity->mXRot);

            level->addEntity(entity);
        }
    }
}

void StructureTemplate::placeInWorld(Level* level, const BlockPos& pos, StructureProcessor* processor,
                                     StructurePlaceSettings* settings, int flags) const
{
    if ((mBlocks.empty() && (settings->isIgnoreEntities() || mEntities.empty())) || mSize.x < 1 || mSize.y < 1
        || mSize.z < 1)
    {
        delete processor;
        return;
    }

    Block* ignoreBlock = settings->getIgnoreBlock();
    BoundingBox* boundingBox = settings->getBoundingBox();

    BlockPos lastColumn(INT_MAX, INT_MAX, INT_MAX);

    for (std::vector<StructureBlockInfo*>::const_iterator it = mBlocks.begin(); it != mBlocks.end(); ++it)
    {
        StructureBlockInfo* templateInfo = *it;

        BlockPos blockPos = calculateRelativePosition(settings, templateInfo->mPos).offset(pos);

        StructureBlockInfo* blockInfo = processor != NULL ? processor->processBlock(level, blockPos, templateInfo)
                                                          : templateInfo;

        if (lastColumn.x != blockPos.x || lastColumn.z != blockPos.z)
        {
            if (!level->hasChunkAt(blockPos) || level->getChunkAt(blockPos)->isEmpty())
            {
                continue;
            }

            lastColumn = blockPos;
        }

        if (blockInfo == NULL)
        {
            continue;
        }

        if (boundingBox != NULL && !boundingBox->isInside(blockPos))
        {
            continue;
        }

        const BlockState* state = blockInfo->mState->getBlock()->mirror(blockInfo->mState, settings->getMirror());
        state = state->getBlock()->rotate(state, settings->getRotation());
        state = settings->getSwappedBlock(state);

        Block* block = state->getBlock();

        if ((ignoreBlock != NULL && ignoreBlock == block)
            || (settings->isIgnoreStructureBlocks() && block == Blocks::structure_block))
        {
            continue;
        }

        if (settings->isPlacingWaterBelowSeaLevel() && blockPos.y < level->getSeaLevel())
        {
            if (Block::getId(block) == BlockId::AIR)
            {
                block = Blocks::water;
                state = block->defaultBlockState();
            }
            else if (block->canContainLiquid(state))
            {
                level->setExtraBlock(blockPos, Blocks::flowing_water->defaultBlockState(), Block::UPDATE_INVISIBLE,
                                     false);
            }
        }

        if (blockInfo->mNbt != NULL)
        {
            boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(blockPos);

            if (blockEntity)
            {
                if (eTYPE_DERIVED_FROM(BlockEntityType::CONTAINER, blockEntity->GetType()))
                {
                    boost::dynamic_pointer_cast<Container>(blockEntity)->clearContent();
                }

                level->setBlock(blockPos, Blocks::barrier->defaultBlockState(), Block::UPDATE_INVISIBLE, false);
            }
        }

        if (level->setBlockAndData(blockPos.x, blockPos.y, blockPos.z, Block::getId(block),
                                   block->convertBlockStateToLegacyData(state), flags)
            && blockInfo->mNbt != NULL)
        {
            boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(blockPos);

            if (blockEntity)
            {
                blockInfo->mNbt->putInt(L"x", blockPos.x);
                blockInfo->mNbt->putInt(L"y", blockPos.y);
                blockInfo->mNbt->putInt(L"z", blockPos.z);

                blockEntity->load(blockInfo->mNbt);
                blockEntity->mirror(settings->getMirror());
                blockEntity->rotate(settings->getRotation());
            }
        }
    }

    for (std::vector<StructureBlockInfo*>::const_iterator it = mBlocks.begin(); it != mBlocks.end(); ++it)
    {
        StructureBlockInfo* blockInfo = *it;

        if (ignoreBlock != NULL && ignoreBlock == blockInfo->mState->getBlock())
        {
            continue;
        }

        BlockPos blockPos = calculateRelativePosition(settings, blockInfo->mPos).offset(pos);

        if (boundingBox != NULL && !boundingBox->isInside(blockPos))
        {
            continue;
        }

        level->blockUpdated(blockPos, blockInfo->mState->getBlock(), false);

        if (blockInfo->mNbt != NULL)
        {
            boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(blockPos);

            if (blockEntity)
            {
                blockEntity->setChanged();
            }
        }
    }

    if (!settings->isIgnoreEntities())
    {
        placeEntities(level, pos, settings->getMirror(), settings->getRotation(), boundingBox);
    }

    delete processor;
}

void StructureTemplate::placeInWorld(Level* level, const BlockPos& pos, StructurePlaceSettings* settings) const
{
    placeInWorld(level, pos, new BlockRotProcessor(settings, pos), settings, 2);
}

void StructureTemplate::placeInWorldChunk(Level* level, const BlockPos& pos, StructurePlaceSettings* settings) const
{
    settings->updateBoundingBoxFromChunkPos();

    placeInWorld(level, pos, settings);
}

void StructureTemplate::placeInWorld(Level* level, const BlockPos& pos, StructurePlaceSettings* settings,
                                     int flags) const
{
    placeInWorld(level, pos, new BlockRotProcessor(settings, pos), settings, flags);
}

BlockPos StructureTemplate::getSize(Rotation* rotation) const
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise90:
    case Rotation::eTYPE_CounterClockwise90:
        return BlockPos(mSize.z, mSize.y, mSize.x);

    default:
        return mSize;
    }
}

BlockPos StructureTemplate::getZeroPositionWithTransform(const BlockPos& pos, Mirror* mirror, Rotation* rotation,
                                                         int sizeX, int sizeZ)
{
    --sizeX;
    --sizeZ;

    int mirroredX = mirror == Mirror::FRONT_BACK ? sizeX : 0;
    int mirroredZ = mirror == Mirror::LEFT_RIGHT ? sizeZ : 0;

    BlockPos zeroPos = pos;

    switch (rotation->getType())
    {
    case Rotation::eTYPE_None:
        zeroPos = pos.offset(mirroredX, 0, mirroredZ);
        break;

    case Rotation::eTYPE_Clockwise90:
        zeroPos = pos.offset(sizeZ - mirroredZ, 0, mirroredX);
        break;

    case Rotation::eTYPE_Clockwise180:
        zeroPos = pos.offset(sizeX - mirroredX, 0, sizeZ - mirroredZ);
        break;

    case Rotation::eTYPE_CounterClockwise90:
        zeroPos = pos.offset(mirroredZ, 0, sizeX - mirroredX);
        break;

    default:
        break;
    }

    return zeroPos;
}

BlockPos StructureTemplate::getZeroPositionWithTransform(const BlockPos& pos, Mirror* mirror, Rotation* rotation) const
{
    return getZeroPositionWithTransform(pos, mirror, rotation, getSize().x, getSize().z);
}

void StructureTemplate::addWalkers(DataFixerUpper* fixerUpper)
{
    fixerUpper->addWalker(DataFixTypes::eDataFixTypes_Structure, (DataWalker*)new AnonymousDataWalker());
}

void StructureTemplate::load(CompoundTag* tag)
{
    for (std::vector<StructureBlockInfo*>::iterator block = mBlocks.begin(); block != mBlocks.end(); ++block)
    {
        delete *block;
    }

    for (std::vector<StructureEntityInfo*>::iterator entity = mEntities.begin(); entity != mEntities.end(); ++entity)
    {
        delete *entity;
    }

    mEntities.clear();
    mPalette.clear();
    mBlocks.clear();

    ListTag* sizeTag = tag->getList(SIZE_TAG, Tag::TAG_INT);
    mSize = BlockPos(sizeTag->getInt(0), sizeTag->getInt(1), sizeTag->getInt(2));

    mAuthor = tag->getString(AUTHOR_TAG);

    ListTag* paletteTag = tag->getList(PALETTE_TAG, Tag::TAG_COMPOUND);

    for (int i = 0; i < paletteTag->size(); ++i)
    {
        const BlockState* state = NbtUtils::readBlockState(paletteTag->getCompound(i));
        mPalette.push_back(state);
    }

    ListTag* blocksTag = tag->getList(BLOCKS_TAG, Tag::TAG_COMPOUND);

    for (int i = 0; i < blocksTag->size(); ++i)
    {
        CompoundTag* blockTag = blocksTag->getCompound(i);

        ListTag* posTag = blockTag->getList(BLOCK_POS_TAG, Tag::TAG_INT);
        BlockPos blockPos(posTag->getInt(0), posTag->getInt(1), posTag->getInt(2));

        unsigned int paletteIndex = (unsigned int)blockTag->getInt(BLOCK_STATE_TAG);

        const BlockState* state = paletteIndex < mPalette.size() ? mPalette.at(paletteIndex)
                                                                 : Blocks::air->defaultBlockState();

        CompoundTag* blockNbt = blockTag->contains(BLOCK_NBT_TAG) ? blockTag->getCompound(BLOCK_NBT_TAG)->copy() : NULL;

        mBlocks.push_back(new StructureBlockInfo(blockPos, state, blockNbt));
    }

    std::sort(mBlocks.begin(), mBlocks.end(), StructureBlockInfo::byColumnSorter());

    ListTag* entitiesTag = tag->getList(ENTITIES_TAG, Tag::TAG_COMPOUND);

    for (int i = 0; i < entitiesTag->size(); ++i)
    {
        CompoundTag* entityTag = entitiesTag->getCompound(i);

        ListTag* posTag = entityTag->getList(ENTITY_POS_TAG, Tag::TAG_DOUBLE);
        Vec3* entityPos = Vec3::newPermanent(posTag->getDouble(0), posTag->getDouble(1), posTag->getDouble(2));

        ListTag* blockPosTag = entityTag->getList(ENTITY_BLOCK_POS_TAG, Tag::TAG_INT);
        BlockPos blockPos(blockPosTag->getInt(0), blockPosTag->getInt(1), blockPosTag->getInt(2));

        if (entityTag->contains(ENTITY_NBT_TAG))
        {
            CompoundTag* entityNbt = entityTag->getCompound(ENTITY_NBT_TAG)->copy();

            mEntities.push_back(new StructureEntityInfo(entityPos, blockPos, entityNbt));
        }
    }
}
