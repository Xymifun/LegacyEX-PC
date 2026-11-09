#include "game/world/generation/CoralFeature.hpp"
#include "game/core/CoralFan.hpp"
#include "game/core/CoralFanHang.hpp"
#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/CoralBlock.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"


CoralFeature::~CoralFeature()
{
}

const Direction* CoralFeature::_randomDirectionExcept(Random& random, const Direction* except)
{
    if (except == Direction::NORTH)
    {
        static const Direction* notNorth[3] = {Direction::EAST, Direction::SOUTH, Direction::WEST};
        return notNorth[random.nextInt(3)];
    }
    if (except == Direction::SOUTH)
    {
        static const Direction* notSouth[3] = {Direction::EAST, Direction::NORTH, Direction::WEST};
        return notSouth[random.nextInt(3)];
    }
    if (except == Direction::EAST)
    {
        static const Direction* notEast[3] = {Direction::NORTH, Direction::SOUTH, Direction::WEST};
        return notEast[random.nextInt(3)];
    }
    static const Direction* notWest[3] = {Direction::EAST, Direction::SOUTH, Direction::NORTH};
    return notWest[random.nextInt(3)];
}

const Direction* CoralFeature::_randomDirectionExcept(Random& random, const Direction* first, const Direction* second)
{
    if ((first == Direction::NORTH && second == Direction::EAST)
        || (first == Direction::EAST && second == Direction::NORTH))
    {
        static const Direction* southWest[2] = {Direction::SOUTH, Direction::WEST};
        return southWest[random.nextInt(2)];
    }
    if ((first == Direction::NORTH && second == Direction::SOUTH)
        || (first == Direction::SOUTH && second == Direction::NORTH))
    {
        static const Direction* eastWest[2] = {Direction::EAST, Direction::WEST};
        return eastWest[random.nextInt(2)];
    }
    if ((first == Direction::NORTH && second == Direction::WEST)
        || (first == Direction::WEST && second == Direction::NORTH))
    {
        static const Direction* southEast[2] = {Direction::SOUTH, Direction::EAST};
        return southEast[random.nextInt(2)];
    }
    if ((first == Direction::EAST && second == Direction::SOUTH)
        || (first == Direction::SOUTH && second == Direction::EAST))
    {
        static const Direction* northWest[2] = {Direction::NORTH, Direction::WEST};
        return northWest[random.nextInt(2)];
    }
    if ((first == Direction::EAST && second == Direction::WEST)
        || (first == Direction::WEST && second == Direction::EAST))
    {
        static const Direction* southNorth[2] = {Direction::SOUTH, Direction::NORTH};
        return southNorth[random.nextInt(2)];
    }
    static const Direction* northEast[2] = {Direction::NORTH, Direction::EAST};
    return northEast[random.nextInt(2)];
}

bool CoralFeature::_canOverwrite(Level* level, const BlockPos& pos, int data)
{
    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() == Blocks::coral_block && state->getValue(CoralBlock::VARIANT) != data)
        return false;
    return true;
}

bool CoralFeature::_setBlock(Level* level, const BlockPos& pos, const BlockState* state, int data)
{
    if (_canOverwrite(level, pos, data))
    {
        setBlock(level, pos, state, Block::UPDATE_NONE);
        return true;
    }
    return false;
}

bool CoralFeature::_setBlockOnSolid(Level* level, const BlockPos& pos, const BlockState* state, int data)
{
    if (_canOverwrite(level, pos, data) && level->isSolidBlockingBlock(pos.below()))
    {
        setBlock(level, pos, state, Block::UPDATE_NONE);
        return true;
    }
    return false;
}

void CoralFeature::_starFormation(Level* level, Random& random, const BlockPos& pos, const BlockState* state,
                                  float chance, int distance, bool inverted)
{
    const int data = state->getValue(CoralBlock::VARIANT);
    if (!_setBlock(level, pos, state, data))
    {
        return;
    }

    if (inverted)
    {
        if (random.nextFloat() < chance)
        {
            if (!_setBlockOnSolid(level, BlockPos(pos.x + distance, pos.y, pos.z), state, data))
            {
                return;
            }
        }
        if (random.nextFloat() < chance)
        {
            if (!_setBlockOnSolid(level, BlockPos(pos.x - distance, pos.y, pos.z), state, data))
            {
                return;
            }
        }
        if (random.nextFloat() < chance)
        {
            if (!_setBlockOnSolid(level, BlockPos(pos.x, pos.y, pos.z + distance), state, data))
            {
                return;
            }
        }
        if (random.nextFloat() < chance)
        {
            _setBlockOnSolid(level, BlockPos(pos.x, pos.y, pos.z - distance), state, data);
        }
    }
    else
    {
        if (random.nextFloat() > chance)
        {
            if (!_setBlockOnSolid(level, BlockPos(pos.x + distance, pos.y, pos.z), state, data))
            {
                return;
            }
        }
        if (random.nextFloat() > chance)
        {
            if (!_setBlockOnSolid(level, BlockPos(pos.x - distance, pos.y, pos.z), state, data))
            {
                return;
            }
        }
        if (random.nextFloat() > chance)
        {
            if (!_setBlockOnSolid(level, BlockPos(pos.x, pos.y, pos.z + distance), state, data))
            {
                return;
            }
        }
        if (random.nextFloat() > chance)
        {
            _setBlockOnSolid(level, BlockPos(pos.x, pos.y, pos.z - distance), state, data);
        }
    }
}

void CoralFeature::_starCorners(Level* level, Random& random, const BlockPos& pos, const BlockState* state,
                                float chance, int distance, bool inverted)
{
    const int data = state->getValue(CoralBlock::VARIANT);
    if (inverted)
    {
        if (random.nextFloat() < chance)
        {
            if (!_setBlockOnSolid(level, BlockPos(pos.x + distance, pos.y, pos.z + distance), state, data))
            {
                return;
            }
        }
        if (random.nextFloat() < chance)
        {
            if (!_setBlockOnSolid(level, BlockPos(pos.x - distance, pos.y, pos.z + distance), state, data))
            {
                return;
            }
        }
        if (random.nextFloat() < chance)
        {
            if (!_setBlockOnSolid(level, BlockPos(pos.x - distance, pos.y, pos.z + distance), state, data))
            {
                return;
            }
        }
        if (random.nextFloat() < chance)
        {
            _setBlockOnSolid(level, BlockPos(pos.x + distance, pos.y, pos.z - distance), state, data);
        }
    }
    else
    {
        if (random.nextFloat() > chance)
        {
            if (!_setBlockOnSolid(level, BlockPos(pos.x + distance, pos.y, pos.z + distance), state, data))
            {
                return;
            }
        }
        if (random.nextFloat() > chance)
        {
            if (!_setBlockOnSolid(level, BlockPos(pos.x - distance, pos.y, pos.z + distance), state, data))
            {
                return;
            }
        }
        if (random.nextFloat() > chance)
        {
            if (!_setBlockOnSolid(level, BlockPos(pos.x - distance, pos.y, pos.z + distance), state, data))
            {
                return;
            }
        }
        if (random.nextFloat() > chance)
        {
            _setBlockOnSolid(level, BlockPos(pos.x + distance, pos.y, pos.z - distance), state, data);
        }
    }
}

void CoralFeature::_buildPlantArm(Level* level, Random& random, const BlockPos& pos, const BlockState* state,
                                  std::vector<BlockPos>& positions,
                                  std::vector<std::pair<BlockPos, const Direction*> >& sides,
                                  const Direction* direction, bool lastArm)
{
    const int data = state->getValue(CoralBlock::VARIANT);
    const int shape = lastArm ? random.nextInt(1, 3) : random.nextInt(3);
    BlockPos arm = pos;

    if (shape == 0)
    {
        if (!level->isUnderWater(arm.above(1), state) || !_setBlock(level, arm.above(0), state, data))
            return;
        arm = arm.relative(direction);
        if (!level->isUnderWater(arm.above(2), state) || !_setBlock(level, arm.above(1), state, data))
            return;
        arm = arm.relative(direction);
        arm = arm.above();
        arm = arm.above();
        if (!level->isUnderWater(arm.above(), state) || !_setBlock(level, arm, state, data))
            return;
        const Direction* side = Direction::from3DDataValue(random.nextInt(2, 6));
        if (random.nextFloat() < 0.4f)
            sides.push_back(std::make_pair(arm, side));
        arm = arm.above();
        if (!level->isUnderWater(arm.above(), state) || !_setBlock(level, arm, state, data))
            return;
        side = Direction::from3DDataValue(random.nextInt(2, 6));
        if (random.nextFloat() < 0.4f)
            sides.push_back(std::make_pair(arm, side));
        arm = arm.above().relative(direction->getOpposite());
        if (level->isUnderWater(arm.above(), state) && _setBlock(level, arm, state, data) && random.nextFloat() < 0.2f)
            positions.push_back(arm.above());
    }
    else if (shape == 1)
    {
        for (int step = 0; step < 2; ++step)
        {
            if (!level->isUnderWater(arm.above(step + 1), state) || !_setBlock(level, arm.above(step), state, data))
                return;
            const Direction* side = Direction::from3DDataValue(random.nextInt(2, 6));
            if (random.nextFloat() < 0.4f)
                sides.push_back(std::make_pair(arm, side));
        }
        arm = arm.relative(direction).above();
        for (int step = 0; step < 3; ++step)
        {
            if (!level->isUnderWater(arm.above(step + 1), state) || !_setBlock(level, arm.above(step), state, data))
                return;
            const Direction* side = Direction::from3DDataValue(random.nextInt(2, 6));
            if (random.nextFloat() < 0.4f)
                sides.push_back(std::make_pair(arm, side));
        }
        if (random.nextFloat() < 0.2f)
            positions.push_back(arm.above(3));
    }
    else if (shape == 2)
    {
        if (!_setBlock(level, arm, state, data))
            return;
        arm = arm.above().relative(direction);
        for (int step = 0; step < 2; ++step)
        {
            if (!level->isUnderWater(arm.above(step + 1), state) || !_setBlock(level, arm.above(step), state, data))
                return;
            const Direction* side = Direction::from3DDataValue(random.nextInt(2, 6));
            if (random.nextFloat() < 0.4f)
                sides.push_back(std::make_pair(arm, side));
        }
        if (random.nextBoolean())
            arm = arm.relative(direction->getClockWise()).above();
        else
            arm = arm.relative(direction->getCounterClockWise()).above();
        for (int step = 0; step < 2; ++step)
        {
            if (!level->isUnderWater(arm.above(step + 1), state) || !_setBlock(level, arm.above(step), state, data))
                return;
            const Direction* side = Direction::from3DDataValue(random.nextInt(2, 6));
            if (random.nextFloat() < 0.4f)
                sides.push_back(std::make_pair(arm, side));
        }
        if (random.nextFloat() < 0.2f)
            positions.push_back(arm.above(2));
    }
}

void CoralFeature::_buildPlantLike(Level* level, const BlockPos& pos, Random& random, std::vector<BlockPos>& positions,
                                   std::vector<std::pair<BlockPos, const Direction*> >& sides, const BlockState* state)
{
    const int data = state->getValue(CoralBlock::VARIANT);
    const int layers = random.nextInt(2) + 1;
    int width = layers * 2 + 1;
    BlockPos base(pos.x, pos.y, pos.z);

    for (int layer = 0; layer <= layers; ++layer)
    {
        if (!level->isUnderWater(base.above(layer + 1), state) || !_setBlock(level, base, state, data))
            return;
        for (int distance = 1; distance < width / 2 + 1; ++distance)
        {
            _starFormation(level, random, base.above(layer), state,
                           (float)distance * 0.1f + 0.5f - (float)layer * 0.15f, layer, true);
            if (layers == 2 && layer == 0)
                _starCorners(level, random, base, state, 0.25f, distance, true);
        }
        width -= 2;
    }

    base = base.above(layers);
    if (!level->isUnderWater(base.above(), state))
        return;

    const Direction* directions[3];
    directions[0] = Direction::from3DDataValue(random.nextInt(2, 6));
    directions[1] = _randomDirectionExcept(random, directions[0]);
    directions[2] = _randomDirectionExcept(random, directions[0], directions[1]);
    if (!_setBlock(level, base, state, data))
        return;

    bool lastArm = false;
    _buildPlantArm(level, random, base.relative(directions[0]), state, positions, sides, directions[0], lastArm);
    const int arms = random.nextInt(2, 4);
    for (int arm = 1; arm < arms; ++arm)
    {
        if (arm == arms - 1)
            lastArm = true;
        if (!_setBlock(level, base.above(arm), state, data))
            return;
        _buildPlantArm(level, random, base.above(arm).relative(directions[arm]), state, positions, sides,
                       directions[arm], lastArm);
    }
}

void CoralFeature::_buildSpire(Level* level, const BlockPos& pos, Random& random, std::vector<BlockPos>& positions,
                               std::vector<std::pair<BlockPos, const Direction*> >& sides, const BlockState* state)
{
    const int data = state->getValue(CoralBlock::VARIANT);
    const int size = random.nextInt(1, 3) * 2;
    const int width = size + 1;
    BlockPos corner(pos.x - width / 2, pos.y, pos.z - width / 2);

    for (int x = 0; x < width; ++x)
    {
        for (int z = 0; z < width; ++z)
        {
            if ((x == 0 && z == 0) || (x == size && z == size) || (x == 0 && z == size) || (z == 0 && x == size))
                continue;
            BlockPos floor(corner.x + x, corner.y, corner.z + z);
            if (random.nextInt(5) != 0 && level->isSolidBlockingBlock(floor.below())
                && !_setBlock(level, floor, state, data))
                return;
        }
    }

    _setBlock(level, corner, state, data);
    BlockPos spire = pos.above();
    if (!level->isUnderWater(spire.above(), state) || !_setBlock(level, spire, state, data))
        return;

    if (random.nextInt(2) != 0 && !_setBlock(level, BlockPos(spire.x + 1, spire.y, spire.z), state, data))
        return;
    if (random.nextInt(3) != 0 && !_setBlock(level, BlockPos(spire.x - 1, spire.y, spire.z), state, data))
        return;
    if (random.nextInt(4) != 0 && !_setBlock(level, BlockPos(spire.x, spire.y, spire.z + 1), state, data))
        return;
    if (random.nextInt(3) != 0 && !_setBlock(level, BlockPos(spire.x, spire.y, spire.z - 1), state, data))
        return;

    spire = spire.above();
    _starFormation(level, random, spire, state, 0.33f, 1, true);
    spire = spire.above();
    _starFormation(level, random, spire, state, 0.7f, 1, true);
    spire = spire.above();

    const int height = random.nextInt(1, 3);
    for (int step = 0; step < height; ++step)
    {
        if (!level->isUnderWater(spire.above(step + 1), state) || !_setBlock(level, spire.above(step), state, data))
            return;
        const Direction* side = Direction::from3DDataValue(random.nextInt(2, 6));
        if (random.nextFloat() < 0.4f)
            sides.push_back(std::make_pair(spire, side));
        if (step == height - 1 && random.nextFloat() < 0.4f)
            positions.push_back(spire.above(height));
    }
}

void CoralFeature::_buildHand(Level* level, const BlockPos& pos, Random& random, std::vector<BlockPos>& positions,
                              std::vector<std::pair<BlockPos, const Direction*> >& sides, const BlockState* state)
{
    const int data = state->getValue(CoralBlock::VARIANT);
    const Direction* direction = Direction::from3DDataValue(random.nextInt(2, 6));
    const Direction* clockWise = direction->getClockWise();
    const Direction* counterClockWise = clockWise->getOpposite();
    const int shape = random.nextInt(3, 5);
    const int height = random.nextInt(1, 4);
    BlockPos palm = pos;

    if (!level->isUnderWater(pos.above(height + 2), state))
        return;
    for (int step = 0; step < height; ++step)
    {
        if (!_setBlock(level, palm.above(step), state, data))
            return;
    }

    palm = palm.above(height);
    BlockPos left = palm;
    BlockPos right = palm;
    if (shape == 3)
    {
        left = left.relative(clockWise);
    }
    else if (random.nextBoolean())
    {
        left = left.relative(clockWise);
        if (!_setBlock(level, left, state, data))
            return;
        left = left.relative(clockWise);
    }
    else
    {
        left = left.relative(clockWise);
        right = right.relative(counterClockWise);
        if (!_setBlock(level, right, state, data))
            return;
    }
    right = right.relative(counterClockWise);
    if (!_setBlock(level, left, state, data) || !_setBlock(level, right, state, data))
        return;

    palm = palm.relative(direction);
    left = left.above().relative(direction);
    right = right.above().relative(direction);
    BlockPos fingers[3] = {palm, left, right};
    for (int finger = 0; finger < 3; ++finger)
    {
        const int length = finger == 0 ? random.nextInt(2, 4) : random.nextInt(4, 6);
        for (int step = 0; step < length; ++step)
        {
            if (!_setBlock(level, fingers[finger], state, data))
                return;
            if (random.nextFloat() < 0.1f)
                positions.push_back(fingers[finger].above());
            if (random.nextFloat() < 0.6f)
                sides.push_back(std::make_pair(fingers[finger], clockWise));
            if (random.nextFloat() < 0.6f)
                sides.push_back(std::make_pair(fingers[finger], counterClockWise));
            if (random.nextFloat() < 0.25f && level->isUnderWater(fingers[finger].above(2), state))
                fingers[finger] = fingers[finger].above();
            fingers[finger] = fingers[finger].relative(direction);
        }
    }
}

void CoralFeature::_buildSmallClump(Level* level, const BlockPos& pos, Random& random, std::vector<BlockPos>& positions,
                                    std::vector<std::pair<BlockPos, const Direction*> >& sides, const BlockState* state)
{
    const int data = state->getValue(CoralBlock::VARIANT);
    BlockPos corner(pos.x - 1, pos.y, pos.z - 1);
    for (int x = 0; x < 3; ++x)
    {
        for (int z = 0; z < 3; ++z)
        {
            BlockPos floor(corner.x + x, corner.y, corner.z + z);
            if (random.nextInt(3) != 0 && !level->isEmptyWaterBlock(floor.below())
                && !_setBlock(level, floor, state, data))
                return;
        }
    }

    BlockPos top = pos.above();
    if (!level->isUnderWater(top.above(), state) || !_setBlock(level, top, state, data))
        return;

    const BlockPos neighbours[4] = {BlockPos(top.x + 1, top.y, top.z), BlockPos(top.x - 1, top.y, top.z),
                                    BlockPos(top.x, top.y, top.z + 1), BlockPos(top.x, top.y, top.z - 1)};
    for (int i = 0; i < 4; ++i)
    {
        if (random.nextInt(4) == 0 || level->isEmptyWaterBlock(neighbours[i].below()))
            continue;
        if (!_setBlock(level, neighbours[i], state, data))
            return;
        if (random.nextFloat() < 0.2f)
            positions.push_back(neighbours[i].above());
    }

    if (level->isUnderWater(top.above(2), state))
    {
        if (!_setBlock(level, top.above(), state, data))
            return;
        if (random.nextFloat() < 0.2f)
            positions.push_back(top.above(2));
    }

    if (random.nextFloat() < 0.25f && level->isUnderWater(top.above(3), state)
        && _setBlock(level, top.above(2), state, data) && random.nextFloat() < 0.1f)
        positions.push_back(top.above(3));
}

void CoralFeature::_placeCoral(Level* level, const BlockPos& pos, Random& random, std::vector<BlockPos>& positions,
                               std::vector<std::pair<BlockPos, const Direction*> >& sides, int variant)
{
    const int shape = random.nextInt(3);
    TypedBoxed<int> variantValue(variant);
    const BlockState* state = Blocks::coral_block->defaultBlockState()->setBoxedValue(
        static_cast<const Property*>(CoralBlock::VARIANT), &variantValue);

    switch (shape)
    {
    case 0:
        _buildPlantLike(level, pos, random, positions, sides, state);
    case 1:
        _buildSpire(level, pos, random, positions, sides, state);
    case 2:
        if (random.nextInt(7) == 0)
            _buildHand(level, pos, random, positions, sides, state);
        else
            _buildSmallClump(level, pos, random, positions, sides, state);
        break;
    }
}

void CoralFeature::_placeTopDecorations(Level* level, const BlockPos& pos, Random& random)
{
    if (!level->isEmptyWaterBlock(pos) || level->isEmptyWaterBlock(pos.below()))
        return;
    if (!Blocks::coral->canSurvive(level, pos, level->getBlockState(pos)))
        return;

    const BlockState* state = Blocks::coral_fan->defaultBlockState()
                                  ->setValue(CoralFan::VARIANT, random.nextInt(5))
                                  ->setValue(CoralFan::DIRECTION, random.nextInt(2));
    setBlock(level, pos, state, Block::UPDATE_NONE);
}

bool CoralFeature::_canPlaceHangOn(Level* level, const BlockPos& pos)
{
    Block* block = level->getBlock(pos);
    if (block == Blocks::coral_block || block == Blocks::stone)
        return true;
    return false;
}

int CoralFeature::_turnFacingIntoint(const Direction* direction)
{
    if (direction == Direction::NORTH)
        return 2;
    if (direction == Direction::EAST)
        return 1;
    if (direction == Direction::SOUTH)
        return 3;
    return 0;
}

const BlockState* CoralFeature::_setCoralHangData(int facing, int variant, int type)
{
    const BlockState* state;
    if (type == 0)
    {
        state = Blocks::coral_fan_hang->defaultBlockState();
    }
    else if (type == 1)
    {
        state = Blocks::coral_fan_hang2->defaultBlockState();
    }
    else
    {
        return Blocks::coral_fan_hang3->defaultBlockState()->setValue(CoralFanHang::DIRECTION, facing);
    }

    return state->setValue(CoralFanHang::VARIANT, variant)->setValue(CoralFanHang::DIRECTION, facing);
}

void CoralFeature::_placeSideDecorations(Level* level, const BlockPos& pos, Random& random, const Direction* direction)
{
    bool canHang = _canPlaceHangOn(level, pos);
    const Direction* side = _randomDirectionExcept(random, direction);
    if (!level->isEmptyWaterBlock(pos.relative(side)) || !canHang)
        return;
    if (level->isUnderWater(pos.relative(side), Blocks::coral_fan_hang->defaultBlockState()))
    {
        int facing = _turnFacingIntoint(side);
        int variant = random.nextInt(2);
        int type = random.nextInt(3);
        const BlockState* hang = _setCoralHangData(facing, variant, type);
        setBlock(level, pos.relative(side), hang, Block::UPDATE_NONE);
    }
}

bool CoralFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (level->mIsClientSide)
        return true;

    BlockPos pos = origin;
    while (pos.y < 59 && !level->isEmptyWaterBlock(pos))
        pos = pos.above();
    if (pos.y >= 59)
        return false;
    if (!level->isSolidBlockingBlock(pos.below()))
        return false;
    if (!level->isEmptyWaterBlock(pos))
        return false;

    const BlockState* below = level->getBlockState(pos.below());
    if (below->getBlock() == Blocks::coral_block && !below->getValue(CoralBlock::DEAD))
        return false;

    int variant = random.nextInt(5);
    std::vector<BlockPos> positions;
    std::vector<std::pair<BlockPos, const Direction*> > sides;
    _placeCoral(level, pos, random, positions, sides, variant);
    for (std::vector<BlockPos>::iterator it = positions.begin(); it != positions.end(); ++it)
        _placeTopDecorations(level, *it, random);
    for (std::vector<std::pair<BlockPos, const Direction*> >::iterator it = sides.begin(); it != sides.end(); ++it)
        _placeSideDecorations(level, it->first, random, it->second);
    return true;
}
