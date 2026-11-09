#include "game/world/generation/CoralCrustFeature.hpp"

#include "game/core/CoralFan.hpp"
#include "game/core/CoralFanHang.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Rotation.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/CoralBlock.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"
#include "game/world/generation/StructureTemplate.hpp"

CoralCrustFeature::~CoralCrustFeature()
{
}

BlockPos CoralCrustFeature::getOffsetFromRot(Rotation* rotation, int type) const
{
    if (type == 0)
    {
        if (rotation == Rotation::NONE)
        {
            return BlockPos(6, 0, 6);
        }
        else if (rotation == Rotation::CLOCKWISE_90)
        {
            return BlockPos(-7, 0, 6);
        }
        else if (rotation == Rotation::CLOCKWISE_180)
        {
            return BlockPos(-7, 0, -7);
        }
        else
        {
            return BlockPos(6, 0, -7);
        }
    }
    if (type == 1)
    {
        if (rotation == Rotation::NONE)
        {
            return BlockPos(6, 0, 6);
        }
        else if (rotation == Rotation::CLOCKWISE_90)
        {
            return BlockPos(-6, 0, 6);
        }
        else if (rotation == Rotation::CLOCKWISE_180)
        {
            return BlockPos(-7, 0, -7);
        }
        else
        {
            return BlockPos(6, 0, -7);
        }
    }
    if (type < 4)
    {
        if (rotation == Rotation::NONE)
        {
            return BlockPos(4, 0, 4);
        }
        else if (rotation == Rotation::CLOCKWISE_90)
        {
            return BlockPos(-4, 0, 4);
        }
        else if (rotation == Rotation::CLOCKWISE_180)
        {
            return BlockPos(-4, 0, -4);
        }
        else
        {
            return BlockPos(4, 0, -4);
        }
    }
    else
    {
        if (rotation == Rotation::NONE)
        {
            return BlockPos(3, 0, 6);
        }
        else if (rotation == Rotation::CLOCKWISE_90)
        {
            return BlockPos(-7, 0, 3);
        }
        else if (rotation == Rotation::CLOCKWISE_180)
        {
            return BlockPos(-2, 0, -7);
        }
        else
        {
            return BlockPos(7, 0, -3);
        }
    }
}

BlockPos CoralCrustFeature::getCropOffsetFromRot(Rotation* rotation, int type) const
{
    if (type == 0)
    {
        if (rotation == Rotation::NONE)
        {
            return BlockPos(-5, 0, -2);
        }
        else if (rotation == Rotation::CLOCKWISE_90)
        {
            return BlockPos(3, 0, -5);
        }
        else if (rotation == Rotation::CLOCKWISE_180)
        {
            return BlockPos(5, 0, 2);
        }
        else
        {
            return BlockPos(-2, 0, 5);
        }
    }
    if (type == 1)
    {
        if (rotation == Rotation::NONE)
        {
            return BlockPos(-7, -2, -3);
        }
        else if (rotation == Rotation::CLOCKWISE_90)
        {
            return BlockPos(3, -2, -6);
        }
        else if (rotation == Rotation::CLOCKWISE_180)
        {
            return BlockPos(6, -2, 3);
        }
        else
        {
            return BlockPos(-2, -2, 5);
        }
    }
    if (type == 2)
    {
        if (rotation == Rotation::NONE)
        {
            return BlockPos(-4, 0, -3);
        }
        else if (rotation == Rotation::CLOCKWISE_90)
        {
            return BlockPos(3, 0, -4);
        }
        else if (rotation == Rotation::CLOCKWISE_180)
        {
            return BlockPos(4, 0, 3);
        }
        else
        {
            return BlockPos(-3, 0, 4);
        }
    }
    if (type == 3)
    {
        if (rotation == Rotation::NONE)
        {
            return BlockPos(-3, 0, -3);
        }
        else if (rotation == Rotation::CLOCKWISE_90)
        {
            return BlockPos(3, 0, -3);
        }
        else if (rotation == Rotation::CLOCKWISE_180)
        {
            return BlockPos(3, 0, 3);
        }
        else
        {
            return BlockPos(-3, 0, 3);
        }
    }
    if (type == 4)
    {
        if (rotation == Rotation::NONE)
        {
            return BlockPos(-2, 0, -2);
        }
        else if (rotation == Rotation::CLOCKWISE_90)
        {
            return BlockPos(2, 0, -2);
        }
        else if (rotation == Rotation::CLOCKWISE_180)
        {
            return BlockPos(2, 0, 2);
        }
        else
        {
            return BlockPos(-2, 0, 2);
        }
    }
    if (type != 5)
    {
        return BlockPos();
    }
    if (rotation == Rotation::NONE)
    {
        return BlockPos(-5, 0, -5);
    }
    if (rotation == Rotation::CLOCKWISE_90)
    {
        return BlockPos(6, 0, -6);
    }
    if (rotation == Rotation::CLOCKWISE_180)
    {
        return BlockPos(5, 0, 5);
    }
    return BlockPos(-5, 0, 5);
}

const BlockState* CoralCrustFeature::setColorData(Level* level, const BlockState* state, int color) const
{
    return state->setValue(CoralBlock::VARIANT, color);
}

bool CoralCrustFeature::canOverwrite(Level* level, const BlockPos& pos)
{
    return level->getBlock(pos) != Blocks::coral_block;
}

void CoralCrustFeature::placeCoral(Level* level, const BlockPos& pos, Random& random, std::vector<BlockPos>& positions,
                                   std::vector<std::pair<BlockPos, const Direction*> >& sides, int type)
{
    const int shape = random.nextInt(4);
    BlockPos position = pos;
    if (shape == 0)
    {
        if (!level->isUnderWater(pos, 259, 0))
        {
            return;
        }

        const BlockState* state = setColorData(level, Blocks::coral_block->defaultBlockState(), type);
        if (canOverwrite(level, position.east()))
        {
            setBlock(level, position.east(), state, Block::UPDATE_NONE);
        }
        if (canOverwrite(level, position.north()))
        {
            setBlock(level, position.north(), state, Block::UPDATE_NONE);
        }
        if (canOverwrite(level, position.west()))
        {
            setBlock(level, position.west(), state, Block::UPDATE_NONE);
        }
        if (canOverwrite(level, position.south()))
        {
            setBlock(level, position.south(), state, Block::UPDATE_NONE);
        }
        setBlock(level, position, state, Block::UPDATE_NONE);
        if (setBlock(level, position.above(), state, Block::UPDATE_NONE) && random.nextInt(4) == 0)
        {
            positions.push_back(position.above(2));
        }
        return;
    }

    const int length = random.nextInt(1, 4);
    const BlockState* state = setColorData(level, Blocks::coral_block->defaultBlockState(), type);
    BlockPos current = position;
    if (!level->isEmptyWaterBlock(current))
    {
        return;
    }

    setBlock(level, current, state, Block::UPDATE_NONE);
    int axis = random.nextInt(2);
    for (int i = 0; i < length; ++i)
    {
        const Direction* side;
        if (axis == 0)
        {
            if (random.nextBoolean())
            {
                current = current.west();
                side = Direction::EAST;
            }
            else
            {
                current = current.east();
                side = Direction::WEST;
            }
        }
        else
        {
            if (random.nextBoolean())
            {
                current = current.south();
                side = Direction::NORTH;
            }
            else
            {
                current = current.north();
                side = Direction::SOUTH;
            }
        }
        axis = (axis + 1) % 2;

        if (!level->isUnderWater(current, 259, 0) || !canOverwrite(level, current)
            || !setBlock(level, current, state, Block::UPDATE_NONE))
        {
            return;
        }

        if (random.nextInt(4) == 0)
        {
            positions.push_back(current.above());
        }
        if (random.nextBoolean())
        {
            sides.push_back(std::make_pair(current, side));
        }
        if (random.nextBoolean())
        {
            sides.push_back(std::make_pair(current, side));
        }
        if (random.nextInt(3) == 0)
        {
            sides.push_back(std::make_pair(current, side));
        }
    }
}

void CoralCrustFeature::placeTopDecorations(Level* level, const BlockPos& pos, Random& random)
{
    if (!level->isEmptyWaterBlock(pos) || level->isEmptyWaterBlock(pos.below())
        || !Blocks::coral->canSurvive(level, pos, NULL))
    {
        return;
    }

    const BlockState* state = Blocks::coral_fan->defaultBlockState();
    state = state->setValue(CoralFan::VARIANT, random.nextInt(CoralFan::VARIANT->getValueCount()));
    AbstractProperty<int>* direction = CoralFan::DIRECTION;
    state = state->setValue(direction, random.nextInt(direction->getValueCount()));
    setBlock(level, pos, state, Block::UPDATE_NONE);
}

bool CoralCrustFeature::canPlaceHangOn(Level* level, const BlockPos& pos)
{
    Block* block = level->getBlock(pos);
    if (block == Blocks::coral_block || block == Blocks::stone)
    {
        return true;
    }
    return false;
}

const Direction* CoralCrustFeature::randomDirectionExcept(Random& random, const Direction* except) const
{
    const Direction* result;
    switch (except->getOrdinal())
    {
    default:
    {
        const Direction* notWest[3] = {Direction::EAST, Direction::SOUTH, Direction::NORTH};
        result = notWest[random.nextInt(3)];
        break;
    }
    case 2:
    {
        const Direction* notNorth[3] = {Direction::EAST, Direction::SOUTH, Direction::WEST};
        return notNorth[random.nextInt(3)];
    }
    case 3:
    {
        const Direction* notSouth[3] = {Direction::EAST, Direction::NORTH, Direction::WEST};
        return notSouth[random.nextInt(3)];
    }
    case 5:
    {
        const Direction* notEast[3] = {Direction::NORTH, Direction::SOUTH, Direction::WEST};
        result = notEast[random.nextInt(3)];
        break;
    }
    }
    return result;
}

int CoralCrustFeature::turnFacingIntoint(const Direction* direction) const
{
    if (direction == Direction::NORTH)
    {
        return 2;
    }
    if (direction == Direction::EAST)
    {
        return 1;
    }
    return direction == Direction::SOUTH ? 3 : 0;
}

const BlockState* CoralCrustFeature::setCoralHangData(int facing, int variant, int type) const
{
    const BlockState* state;
    switch (type)
    {
    case 0:
        state = Blocks::coral_fan_hang->defaultBlockState()->setValue(CoralFanHang::VARIANT, variant);
        break;
    case 1:
        state = Blocks::coral_fan_hang2->defaultBlockState()->setValue(CoralFanHang::VARIANT, variant);
        break;
    default:
        state = Blocks::coral_fan_hang3->defaultBlockState();
        break;
    }

    return state->setValue(CoralFanHang::DIRECTION, facing);
}

void CoralCrustFeature::placeSideDecorations(Level* level, const BlockPos& pos, Random& random,
                                             const Direction* direction)
{
    const bool canHang = canPlaceHangOn(level, pos);
    const Direction* side = randomDirectionExcept(random, direction);
    if (!level->isEmptyWaterBlock(pos.relative(side)) || !canHang)
    {
        return;
    }
    if (!level->isUnderWater(pos.relative(side), 259, 0))
    {
        return;
    }

    const int facing = turnFacingIntoint(side);
    const int variant = random.nextInt(2);
    const int type = random.nextInt(3);
    const BlockState* state = setCoralHangData(facing, variant, type);
    const BlockPos placePos = pos.relative(side);
    setBlock(level, placePos, state, Block::UPDATE_NONE);
}

void CoralCrustFeature::placeCoralBase(Level* level, const BlockPos& pos, Random& random,
                                       const StructureTemplate* structure, StructurePlaceSettings& settings)
{
    StructureTemplate::MarkerMap markers = structure->getMarkers(pos, &settings);
    std::vector<BlockPos> positions;
    std::vector<std::pair<BlockPos, const Direction*> > sides;
    const int type = random.nextInt(5);
    for (StructureTemplate::MarkerMap::iterator it = markers.begin(); it != markers.end(); ++it)
    {
        BlockPos marker = it->first;
        if (level->isSolidBlockingBlock(marker.below()))
        {
            if (level->isSolidBlockingBlock(marker))
            {
                marker = marker.above();
            }
            placeCoral(level, marker, random, positions, sides, type);
        }
    }

    for (std::vector<BlockPos>::iterator it = positions.begin(); it != positions.end(); ++it)
    {
        placeTopDecorations(level, *it, random);
    }
    for (std::vector<std::pair<BlockPos, const Direction*> >::iterator it = sides.begin(); it != sides.end(); ++it)
    {
        placeSideDecorations(level, it->first, random, it->second);
    }
}

bool CoralCrustFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (level->mIsClientSide)
    {
        return true;
    }

    eMinecraftStructureTemplate crustTemplates[5] = {
        eMinecraftStructureTemplate_CoralCrust_Crust1, eMinecraftStructureTemplate_CoralCrust_Crust2,
        eMinecraftStructureTemplate_CoralCrust_Crust3, eMinecraftStructureTemplate_CoralCrust_Crust4,
        eMinecraftStructureTemplate_CoralCrust_Crust5};
    eMinecraftStructureTemplate cropTemplates[6] = {
        eMinecraftStructureTemplate_CoralCrust_Outcropping1, eMinecraftStructureTemplate_CoralCrust_Outcropping2,
        eMinecraftStructureTemplate_CoralCrust_Outcropping3, eMinecraftStructureTemplate_CoralCrust_Outcropping4,
        eMinecraftStructureTemplate_CoralCrust_Outcropping5, eMinecraftStructureTemplate_CoralCrust_Outcropping6};

    std::vector<BlockPos> positions;
    std::vector<int> types;
    std::vector<Rotation*> rotations;
    StructurePlaceSettings settings;
    settings.setIgnoreEntities(true);

    int type = random.nextInt(5);
    Rotation* rotation = Rotation::getRandom(random);
    types.push_back(type);
    rotations.push_back(rotation);

    BlockPos position = origin + getOffsetFromRot(rotations[0], types[0]);
    while (position.y < 54 && !level->isEmptyWaterBlock(position))
    {
        position = position.above();
    }
    if (position.y >= 54)
    {
        return false;
    }
    if (!level->isSolidBlockingBlock(position.below()) || !level->isEmptyWaterBlock(position))
    {
        return false;
    }

    const BlockState* below = level->getBlockState(position.below());
    if (below->getBlock() == Blocks::coral_block && !below->getValue(CoralBlock::DEAD))
    {
        return false;
    }

    positions.push_back(position - getOffsetFromRot(rotations[0], types[0]));

    if (random.nextInt(4) != 0)
    {
        int count = random.nextInt(4, 9) + 1;
        for (int i = 1; i < count; ++i)
        {
            int crustType = random.nextInt(5);
            Rotation* crustRotation = Rotation::getRandom(random);
            int x = random.nextInt(3, 12);
            if (random.nextBoolean())
            {
                x = -x;
            }
            int z = random.nextInt(3, 12);
            if (random.nextBoolean())
            {
                z = -z;
            }

            const BlockPos spread(x, 0, z);
            const BlockPos candidate = position + spread + getOffsetFromRot(crustRotation, crustType);
            if (!level->isSolidBlockingBlock(candidate.below()))
            {
                continue;
            }
            const BlockPos start = candidate - getOffsetFromRot(crustRotation, crustType);
            types.push_back(crustType);
            rotations.push_back(crustRotation);
            positions.push_back(start);
        }

        if (positions.size() == 1)
        {
            Rotation* cropRotation = rotations[0];
            settings.setRotation(cropRotation);
            const int cropType = random.nextInt(6);
            const StructureTemplate* structure = level->getLevelStorage()->getStructureManager()->getOrCreate(
                cropTemplates[cropType]);
            structure->placeInWorld(level, position + getCropOffsetFromRot(cropRotation, cropType), &settings);
            placeCoralBase(level, position + getCropOffsetFromRot(cropRotation, cropType), random, structure, settings);
            return true;
        }
    }
    else
    {
        const int cropType = random.nextInt(6);
        Rotation* cropRotation = rotations[0];
        settings.setRotation(cropRotation);
        const StructureTemplate* structure = level->getLevelStorage()->getStructureManager()->getOrCreate(
            cropTemplates[cropType]);
        structure->placeInWorld(level, position + getCropOffsetFromRot(cropRotation, cropType), &settings);
        placeCoralBase(level, position + getCropOffsetFromRot(cropRotation, cropType), random, structure, settings);
        return true;
    }

    for (unsigned int i = 0; i < positions.size(); ++i)
    {
        settings.setRotation(rotations[i]);
        const StructureTemplate* structure = level->getLevelStorage()->getStructureManager()->getOrCreate(
            crustTemplates[types[i]]);
        structure->placeInWorld(level, positions[i], &settings);
        placeCoralBase(level, positions[i], random, structure, settings);
    }
    return true;
}
