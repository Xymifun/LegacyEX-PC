#include "game/world/generation/StructurePiece.hpp"
#include "game/core/EntityType.hpp"

#include <math.h>
#include <stdlib.h>

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/BoundingBox.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DispenserBlock.hpp"
#include "game/world/block/DispenserBlockEntity.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/RandomizableLockableContainerBlockEntity.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"
#include "game/core/StringUtils.hpp"

template <class T>
std::wstring _toString(T value);

template <class T>
T _fromString(const std::wstring& text);

StructurePiece::StructurePiece()
    : mBoundingBox(NULL),
      mOrientation(NULL),
      mMirror(Mirror::NONE),
      mRotation(Rotation::NONE),
      mGenDepth(0),
      mDirty(true)
{
}

StructurePiece::StructurePiece(int genDepth) : mGenDepth(genDepth)
{
    mBoundingBox = NULL;
    mOrientation = NULL;
    mMirror = Mirror::NONE;
    mRotation = Rotation::NONE;
    mDirty = true;
}

StructurePiece::~StructurePiece()
{
    delete mBoundingBox;
}

CompoundTag* StructurePiece::createTag()
{
    return new CompoundTag();
}

void StructurePiece::write(DataOutputStream* output)
{
    output->writeUTF(StructureFeatureIO::getEncodeId(this));
    mBoundingBox->write(output);
    output->writeInt(mOrientation != NULL ? mOrientation->get2DDataValue() : -1);
    output->writeInt(mGenDepth);
    addAdditionalSaveData(output);
}

void StructurePiece::setBoundingBox(BoundingBox* boundingBox)
{
    delete mBoundingBox;
    mBoundingBox = boundingBox;
}

void StructurePiece::fromOrientationSetRotationAndMirror(const Direction* orientation, Rotation*& rotation,
                                                         Mirror*& mirror)
{
    if (orientation == NULL)
    {
        rotation = Rotation::NONE;
        mirror = Mirror::NONE;
    }
    else if (orientation == Direction::SOUTH)
    {
        mirror = Mirror::LEFT_RIGHT;
        rotation = Rotation::NONE;
    }
    else if (orientation == Direction::WEST)
    {
        mirror = Mirror::LEFT_RIGHT;
        rotation = Rotation::CLOCKWISE_90;
    }
    else if (orientation == Direction::EAST)
    {
        mirror = Mirror::NONE;
        rotation = Rotation::CLOCKWISE_90;
    }
    else
    {
        mirror = Mirror::NONE;
        rotation = Rotation::NONE;
    }
}

void StructurePiece::setOrientation(const Direction* orientation)
{
    mOrientation = orientation;
    fromOrientationSetRotationAndMirror(orientation, mRotation, mMirror);
}

void StructurePiece::load(Level* level, DataInputStream* input)
{
    delete mBoundingBox;
    mBoundingBox = new BoundingBox();
    mBoundingBox->read(input);

    int data2D = input->readInt();
    setOrientation(data2D == -1 ? NULL : Direction::from2DDataValue(data2D));

    mGenDepth = input->readInt();

    readAdditionalSaveData(input, level->getLevelStorage()->getStructureManager());
}

void StructurePiece::addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random)
{
}

BoundingBox* StructurePiece::getBoundingBox()
{
    return mBoundingBox;
}

int StructurePiece::getGenDepth()
{
    return mGenDepth;
}

void StructurePiece::setGenDepth(int genDepth)
{
    mGenDepth = genDepth;
}

StructurePiece* StructurePiece::findCollisionPiece(std::vector<StructurePiece*>* pieces, BoundingBox* box)
{
    for (std::vector<StructurePiece*>::iterator it = pieces->begin(); it != pieces->end(); ++it)
    {
        StructurePiece* piece = *it;
        if (piece->getBoundingBox() != NULL && piece->getBoundingBox()->intersects(box))
        {
            return piece;
        }
    }

    return NULL;
}

BlockPos* StructurePiece::getLocatorPosition()
{
    return new BlockPos(mBoundingBox->getCenter());
}

bool StructurePiece::edgesLiquid(Level* level, BoundingBox* box)
{
    int x0 = Math::_max(mBoundingBox->x0 - 1, box->x0);
    int y0 = Math::_max(mBoundingBox->y0 - 1, box->y0);
    int z0 = Math::_max(mBoundingBox->z0 - 1, box->z0);
    int x1 = Math::_min(mBoundingBox->x1 + 1, box->x1);
    int y1 = Math::_min(mBoundingBox->y1 + 1, box->y1);
    int z1 = Math::_min(mBoundingBox->z1 + 1, box->z1);

    for (int x = x0; x <= x1; ++x)
    {
        for (int z = z0; z <= z1; ++z)
        {
            if (level->getBlockState(BlockPos(x, y0, z))->getBlock()->mMaterial->isLiquid())
            {
                return true;
            }

            if (level->getBlockState(BlockPos(x, y1, z))->getBlock()->mMaterial->isLiquid())
            {
                return true;
            }
        }
    }

    for (int x = x0; x <= x1; ++x)
    {
        for (int y = y0; y <= y1; ++y)
        {
            if (level->getBlockState(BlockPos(x, y, z0))->getBlock()->mMaterial->isLiquid())
            {
                return true;
            }

            if (level->getBlockState(BlockPos(x, y, z1))->getBlock()->mMaterial->isLiquid())
            {
                return true;
            }
        }
    }

    for (int z = z0; z <= z1; ++z)
    {
        for (int y = y0; y <= y1; ++y)
        {
            if (level->getBlockState(BlockPos(x0, y, z))->getBlock()->mMaterial->isLiquid())
            {
                return true;
            }

            if (level->getBlockState(BlockPos(x1, y, z))->getBlock()->mMaterial->isLiquid())
            {
                return true;
            }
        }
    }

    return false;
}

const Direction* StructurePiece::getOrientation()
{
    return mOrientation;
}

int StructurePiece::getWorldX(int x, int z)
{
    const Direction* orientation = getOrientation();
    if (orientation == NULL)
    {
        return x;
    }

    switch (orientation->get3DDataValue())
    {
    case 2:
    case 3:
        return mBoundingBox->x0 + x;

    case 4:
        return mBoundingBox->x1 - z;

    case 5:
        return mBoundingBox->x0 + z;

    default:
        return x;
    }
}

int StructurePiece::getWorldY(int y)
{
    if (getOrientation() == NULL)
    {
        return y;
    }

    return y + mBoundingBox->y0;
}

int StructurePiece::getWorldZ(int x, int z)
{
    const Direction* orientation = getOrientation();
    if (orientation == NULL)
    {
        return z;
    }

    switch (orientation->get3DDataValue())
    {
    case 2:
        return mBoundingBox->z1 - z;

    case 3:
        return mBoundingBox->z0 + z;

    case 4:
    case 5:
        return mBoundingBox->z0 + x;

    default:
        return z;
    }
}

void StructurePiece::placeBlock(Level* level, const BlockState* state, int x, int y, int z, BoundingBox* box)
{
    BlockPos pos(getWorldX(x, z), getWorldY(y), getWorldZ(x, z));
    if (!box->isInside(pos) || pos.y == 0)
    {
        return;
    }

    if (mMirror != Mirror::NONE)
    {
        state = state->getBlock()->mirror(state, mMirror);
    }

    if (mRotation != Rotation::NONE)
    {
        state = state->getBlock()->rotate(state, mRotation);
    }

    level->setBlock(pos, state, Block::UPDATE_CLIENTS | Block::UPDATE_NO_OBSERVERS, false);
}

const BlockState* StructurePiece::getBlock(Level* level, int x, int y, int z, BoundingBox* box)
{
    BlockPos pos(getWorldX(x, z), getWorldY(y), getWorldZ(x, z));
    if (!box->isInside(pos))
    {
        return Blocks::air->defaultBlockState();
    }

    return level->getBlockState(pos);
}

bool StructurePiece::isAboveGround(Level* level, int x, int y, int z)
{
    BlockPos pos(getWorldX(x, z), getWorldY(y + 1), getWorldZ(x, z));
    return level->canSeeSky(pos);
}

int StructurePiece::getSkyBrightness(Level* level, int x, int y, int z, BoundingBox* box)
{
    BlockPos pos(getWorldX(x, z), getWorldY(y + 1), getWorldZ(x, z));
    if (!box->isInside(pos))
    {
        return 15;
    }

    return level->getBrightness(LightLayer::SKY, pos);
}

void StructurePiece::generateAirBox(Level* level, BoundingBox* box, int x0, int y0, int z0, int x1, int y1, int z1)
{
    for (int y = y0; y <= y1; ++y)
    {
        for (int x = x0; x <= x1; ++x)
        {
            for (int z = z0; z <= z1; ++z)
            {
                placeBlock(level, Blocks::air->defaultBlockState(), x, y, z, box);
            }
        }
    }
}

void StructurePiece::generateBox(Level* level, BoundingBox* box, int x0, int y0, int z0, int x1, int y1, int z1,
                                 const BlockState* outerState, const BlockState* innerState, bool existingOnly)
{
    for (int y = y0; y <= y1; ++y)
    {
        for (int x = x0; x <= x1; ++x)
        {
            for (int z = z0; z <= z1; ++z)
            {
                if (existingOnly && getBlock(level, x, y, z, box)->getBlock()->mMaterial == Material::AIR)
                {
                    continue;
                }

                if (y != y0 && y != y1 && x != x0 && x != x1 && z != z0 && z != z1)
                {
                    placeBlock(level, innerState, x, y, z, box);
                }
                else
                {
                    placeBlock(level, outerState, x, y, z, box);
                }
            }
        }
    }
}

void StructurePiece::generateBox(Level* level, BoundingBox* box, BoundingBox* fillBox, const BlockState* outerState,
                                 const BlockState* innerState, bool existingOnly)
{
    generateBox(level, box, fillBox->x0, fillBox->y0, fillBox->z0, fillBox->x1, fillBox->y1, fillBox->z1, outerState,
                innerState, existingOnly);
}

void StructurePiece::generateBox(Level* level, BoundingBox* box, int x0, int y0, int z0, int x1, int y1, int z1,
                                 bool existingOnly, Random& random, BlockSelector* selector)
{
    for (int y = y0; y <= y1; ++y)
    {
        for (int x = x0; x <= x1; ++x)
        {
            for (int z = z0; z <= z1; ++z)
            {
                if (existingOnly && getBlock(level, x, y, z, box)->getBlock()->mMaterial == Material::AIR)
                {
                    continue;
                }

                selector->next(random, x, y, z, y == y0 || y == y1 || x == x0 || x == x1 || z == z0 || z == z1);
                placeBlock(level, selector->getNext(), x, y, z, box);
            }
        }
    }
}

void StructurePiece::generateBox(Level* level, BoundingBox* box, BoundingBox* fillBox, bool existingOnly,
                                 Random& random, BlockSelector* selector)
{
    generateBox(level, box, fillBox->x0, fillBox->y0, fillBox->z0, fillBox->x1, fillBox->y1, fillBox->z1, existingOnly,
                random, selector);
}

void StructurePiece::generateMaybeBox(Level* level, BoundingBox* box, Random& random, float chance, int x0, int y0,
                                      int z0, int x1, int y1, int z1, const BlockState* outerState,
                                      const BlockState* innerState, bool existingOnly, bool undergroundOnly)
{
    for (int y = y0; y <= y1; ++y)
    {
        for (int x = x0; x <= x1; ++x)
        {
            for (int z = z0; z <= z1; ++z)
            {
                if (random.nextFloat() > chance)
                {
                    continue;
                }

                if (existingOnly && getBlock(level, x, y, z, box)->getBlock()->mMaterial == Material::AIR)
                {
                    continue;
                }

                if (undergroundOnly && isAboveGround(level, x, y, z))
                {
                    continue;
                }

                if (y == y0 || y == y1 || x == x0 || x == x1 || z == z0 || z == z1)
                {
                    placeBlock(level, outerState, x, y, z, box);
                }
                else
                {
                    placeBlock(level, innerState, x, y, z, box);
                }
            }
        }
    }
}

void StructurePiece::maybeGenerateBlock(Level* level, BoundingBox* box, Random& random, float chance, int x, int y,
                                        int z, const BlockState* state)
{
    if (random.nextFloat() < chance)
    {
        placeBlock(level, state, x, y, z, box);
    }
}

void StructurePiece::generateUpperHalfSphere(Level* level, BoundingBox* box, int x0, int y0, int z0, int x1, int y1,
                                             int z1, const BlockState* state, bool excludeAir)
{
    float zRadius = (float)(z1 - z0 + 1) * 0.5f;
    float zCentre = (float)z0 + zRadius;
    float xRadius = (float)(x1 - x0 + 1) * 0.5f;
    float xCentre = (float)x0 + xRadius;
    float ySpan = (float)(y1 - y0 + 1);

    for (int y = y0; y <= y1; ++y)
    {
        float dy = (float)(y - y0) / ySpan;

        for (int x = x0; x <= x1; ++x)
        {
            float dx = ((float)x - xCentre) / xRadius;

            for (int z = z0; z <= z1; ++z)
            {
                float dz = ((float)z - zCentre) / zRadius;

                if (excludeAir && getBlock(level, x, y, z, box)->getBlock()->mMaterial == Material::AIR)
                {
                    continue;
                }

                if (dx * dx + dy * dy + dz * dz <= 1.05f)
                {
                    placeBlock(level, state, x, y, z, box);
                }
            }
        }
    }
}

void StructurePiece::generateFullSphere(Level* level, BoundingBox* box, int centreX, int centreY, int centreZ,
                                        int radius, const BlockState* shellState, const BlockState* innerState,
                                        bool existingOnly)
{
    for (int y = centreY - radius; y <= centreY + radius; ++y)
    {
        for (int x = centreX - radius; x <= centreX + radius; ++x)
        {
            for (int z = centreZ - radius; z <= centreZ + radius; ++z)
            {
                if (existingOnly && getBlock(level, x, y, z, box) == NULL)
                {
                    continue;
                }

                float dx = (float)abs(x - centreX) + 0.5f;
                float dy = (float)abs(y - centreY) + 0.5f;
                float dz = (float)abs(z - centreZ) + 0.5f;
                float sum = dx * dx + dy * dy;
                sum += dz * dz;
                float distance = sqrtf(sum);

                float delta = distance - (float)radius;
                if (fabsf(delta) <= 0.5f)
                {
                    placeBlock(level, shellState, x, y, z, box);
                }

                if ((float)radius - distance > 0.5f)
                {
                    placeBlock(level, innerState, x, y, z, box);
                }
            }
        }
    }
}

void StructurePiece::generateAirColumnUp(Level* level, int x, int y, int z, BoundingBox* box)
{
    BlockPos pos(getWorldX(x, z), getWorldY(y), getWorldZ(x, z));
    if (!box->isInside(pos))
    {
        return;
    }

    while (!level->isEmptyBlock(pos) && pos.y < 255)
    {
        level->setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        pos = pos.above();
    }
}

void StructurePiece::generateAirBoxUp(Level* level, int x0, int z0, int x1, int z1, int y, BoundingBox* box)
{
    for (int x = x0; x < x1; ++x)
    {
        for (int z = z0; z < z1; ++z)
        {
            generateAirColumnUp(level, x, y, z, box);
        }
    }
}

void StructurePiece::fillColumnDown(Level* level, const BlockState* state, int x, int y, int z, BoundingBox* box)
{
    int worldX = getWorldX(x, z);
    int worldY = getWorldY(y);
    int worldZ = getWorldZ(x, z);

    if (!box->isInside(BlockPos(worldX, worldY, worldZ)))
    {
        return;
    }

    while ((level->isEmptyBlock(BlockPos(worldX, worldY, worldZ))
            || level->getBlockState(BlockPos(worldX, worldY, worldZ))->getBlock()->mMaterial->isLiquid())
           && worldY > 1)
    {
        level->setBlock(BlockPos(worldX, worldY, worldZ), state, Block::UPDATE_CLIENTS, false);
        --worldY;
    }
}

void StructurePiece::fillBoxDown(Level* level, const BlockState* state, int x0, int z0, int x1, int z1, int y,
                                 BoundingBox* box)
{
    for (int x = x0; x < x1; ++x)
    {
        for (int z = z0; z < z1; ++z)
        {
            fillColumnDown(level, state, x, y, z, box);
        }
    }
}

bool StructurePiece::createChest(Level* level, BoundingBox* box, Random& random, int x, int y, int z,
                                 ResourceLocation* lootTable)
{
    BlockPos pos(getWorldX(x, z), getWorldY(y), getWorldZ(x, z));
    return createChest(level, box, random, pos, lootTable, NULL);
}

bool StructurePiece::createChest(Level* level, BoundingBox* box, Random& random, const BlockPos& pos,
                                 ResourceLocation* lootTable, const BlockState* state)
{
    if (box->isInside(pos) && level->getBlockState(pos)->getBlock() != Blocks::chest)
    {
        if (state == NULL)
        {
            state = static_cast<ChestBlock*>(Blocks::chest)->reorient(level, pos, Blocks::chest->defaultBlockState());
        }

        level->setBlock(pos, state, Block::UPDATE_CLIENTS, false);

        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
        if (blockEntity)
        {
            bool isChest = eTYPE_DERIVED_FROM(BlockEntityType::CHEST, blockEntity->GetType());
            if (isChest)
            {
                boost::shared_ptr<ChestBlockEntity> chest = boost::static_pointer_cast<ChestBlockEntity>(blockEntity);
                chest->setLootTable(lootTable, random.nextLong());
            }
        }

        return true;
    }

    return false;
}

bool StructurePiece::createDispenser(Level* level, BoundingBox* box, Random& random, int x, int y, int z,
                                     const Direction* facing, ResourceLocation* lootTable)
{
    const BlockPos& pos = BlockPos(getWorldX(x, z), getWorldY(y), getWorldZ(x, z));
    if (box->isInside(pos) && level->getBlockState(pos)->getBlock() != Blocks::dispenser)
    {
        placeBlock(level, Blocks::dispenser->defaultBlockState()->setValue(DispenserBlock::FACING, facing), x, y, z,
                   box);

        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
        bool isDispenser = eTYPE_DERIVED_FROM(BlockEntityType::DISPENSER, blockEntity->GetType());
        if (isDispenser)
        {
            boost::shared_ptr<DispenserBlockEntity> dispenser = boost::static_pointer_cast<DispenserBlockEntity>(
                blockEntity);
            dispenser->setLootTable(lootTable, random.nextInt());
        }

        return true;
    }

    return false;
}

void StructurePiece::addStructurePieceSaveData(DataOutputStream* output, std::vector<bool> firstBools,
                                               std::vector<bool> secondBools, std::vector<int> firstInts,
                                               std::vector<int> secondInts)
{
    addBoolSaveData(output, firstBools);
    addBoolSaveData(output, secondBools);
    addIntSaveData(output, firstInts);
    addIntSaveData(output, secondInts);
}

void StructurePiece::readStructurePieceSaveData(DataInputStream* input, std::vector<bool>& firstBools,
                                                std::vector<bool>& secondBools, std::vector<int>& firstInts,
                                                std::vector<int>& secondInts)
{
    firstBools = readBoolSaveData(input);
    secondBools = readBoolSaveData(input);
    firstInts = readIntSaveData(input);
    secondInts = readIntSaveData(input);
}

void StructurePiece::addBoolSaveData(DataOutputStream* output, std::vector<bool> values)
{
    std::wstring text = L"";

    for (int i = 0; i < values.size(); ++i)
    {
        text += _toString<bool>(values[i]);

        if (i < values.size() - 1)
        {
            text += L",";
        }
    }

    output->writeUTF(text);
}

std::vector<bool> StructurePiece::readBoolSaveData(DataInputStream* input)
{
    std::vector<bool> values;

    std::wstring text = input->readUTF();
    if (!text.empty())
    {
        std::vector<std::wstring> parts;
        parts = stringSplit(text, L',');

        for (int i = 0; i < parts.size(); ++i)
        {
            values.push_back(_fromString<bool>(parts[i]));
        }
    }

    return values;
}

void StructurePiece::addIntSaveData(DataOutputStream* output, std::vector<int> values)
{
    std::wstring text = L"";

    for (int i = 0; i < values.size(); ++i)
    {
        text += _toString<int>(values[i]);

        if (i < values.size() - 1)
        {
            text += L",";
        }
    }

    output->writeUTF(text);
}

std::vector<int> StructurePiece::readIntSaveData(DataInputStream* input)
{
    std::vector<int> values;

    std::wstring text = input->readUTF();
    if (!text.empty())
    {
        std::vector<std::wstring> parts;
        parts = stringSplit(text, L',');

        for (int i = 0; i < parts.size(); ++i)
        {
            values.push_back(_fromString<int>(parts[i]));
        }
    }

    return values;
}

void StructurePiece::createDoor(Level* level, BoundingBox* box, Random& random, int x, int y, int z,
                                const Direction* facing, DoorBlock* door)
{
    placeBlock(level, door->defaultBlockState()->setValue(DoorBlock::FACING, facing), x, y, z, box);
    placeBlock(
        level,
        door->defaultBlockState()->setValue(DoorBlock::FACING, facing)->setValue(DoorBlock::HALF, DoorBlock::UPPER), x,
        y + 1, z, box);
}

void StructurePiece::move(int x, int y, int z)
{
    mBoundingBox->move(x, y, z);
}

Rotation* StructurePiece::getRotation()
{
    return mRotation;
}

void StructurePiece::setRotation(Rotation* rotation)
{
    mRotation = rotation;
}

Mirror* StructurePiece::getMirror()
{
    return mMirror;
}

void StructurePiece::setDirty()
{
    mDirty = true;
}

void StructurePiece::clearDirty()
{
    mDirty = false;
}

bool StructurePiece::isDirty()
{
    return mDirty;
}

StructurePiece::BlockSelector::BlockSelector() : mNext(Blocks::air->defaultBlockState())
{
}

StructurePiece::BlockSelector::~BlockSelector()
{
}

void StructurePiece::BlockSelector::next(Random& random, int x, int y, int z, bool edge)
{
}

const BlockState* StructurePiece::BlockSelector::getNext()
{
    return mNext;
}
