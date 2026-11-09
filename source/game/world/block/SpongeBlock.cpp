#include "game/world/block/SpongeBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/LevelEvent.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

#include <deque>
#include <utility>

AbstractProperty<bool>* SpongeBlock::WET = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"wet"));

SpongeBlock::SpongeBlock() : Block(Material::SPONGE)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(WET, false));
    mWetIcon = NULL;
}

std::wstring SpongeBlock::getName()
{
    return std::wstring(L10N::GetString(IDS_TILE_SPONGE));
}

int SpongeBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(WET) ? 1 : 0;
}

int SpongeBlock::removeWaterBreadthFirstSearch(Level* level, const BlockPos& pos)
{
    std::deque<std::pair<BlockPos, int> > queue;
    std::pair<BlockPos, int> start(pos, 0);
    queue.push_back(start);
    const BlockState* air = Blocks::air->defaultBlockState();
    int removed = 0;
    while (!queue.empty())
    {
        const std::pair<BlockPos, int>& front = queue.front();
        BlockPos currentPos = front.first;
        int distance = front.second;
        queue.pop_front();
        for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin();
             it != Direction::VALUES.end(); ++it)
        {
            BlockPos adjacent = currentPos.relative(*it);
            const BlockState* adjacentState = level->getBlockState(adjacent);
            Block* extraBlock = level->getExtraBlock(adjacent);
            if (adjacentState->getMaterial() == Material::WATER)
            {
                level->setBlock(adjacent, air, Block::UPDATE_ALL, false);
            }
            else if (adjacentState->getBlock() == Blocks::bubble_column_segment)
            {
                level->setExtraBlock(adjacent, air, Block::UPDATE_ALL, false);
                level->setBlock(adjacent, air, Block::UPDATE_ALL, false);
            }
            else if (extraBlock != Blocks::air && level->getExtraBlockState(adjacent)->getMaterial() == Material::WATER)
            {
                level->setExtraBlock(adjacent, air, Block::UPDATE_ALL, false);
            }
            else
            {
                continue;
            }
            ++removed;
            if (distance < 6)
            {
                std::pair<BlockPos, int> next(adjacent, distance + 1);
                queue.push_back(next);
            }
        }
        if (removed > 64)
            break;
    }

    return removed > 0;
}

void SpongeBlock::tryAbsorbWater(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!state->getValue(WET) && removeWaterBreadthFirstSearch(level, pos))
    {
        level->setBlock(pos, state->setValue(WET, true), Block::UPDATE_ALL, false);
        level->levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, pos, Block::getId(Blocks::water));
    }
}

void SpongeBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    tryAbsorbWater(level, pos, state);
}

void SpongeBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                  const BlockPos& fromPos)
{
    tryAbsorbWater(level, pos, state);
    Block::neighborChanged(state, level, pos, block, fromPos);
}

const BlockState* SpongeBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(WET, (data & 1) != 0);
}

int SpongeBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(WET) ? 1 : 0;
}

BlockStateDefinition* SpongeBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {WET};
    return new BlockStateDefinition(this, properties, 1);
}

void SpongeBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    if (!state->getValue(WET))
        return;

    const Direction* direction = Direction::getRandomDirection(*random);
    if (direction == Direction::UP)
        return;

    if (level->getBlockState(pos.relative(direction))->isTopSolidBlocking())
        return;

    double x = pos.x;
    double y = pos.y;
    double z = pos.z;
    if (direction == Direction::DOWN)
    {
        y -= 0.05;
        x += random->nextDouble();
        z += random->nextDouble();
    }
    else
    {
        y += random->nextDouble() * 0.8;
        if (direction->getAxis() == Direction::Axis::X)
        {
            z += random->nextDouble();
            if (direction == Direction::EAST)
            {
                x += 1.1;
            }
            else
            {
                x += 0.05;
            }
        }
        else
        {
            x += random->nextDouble();
            if (direction == Direction::SOUTH)
            {
                z += 1.1;
            }
            else
            {
                z += 0.05;
            }
        }
    }

    level->addParticle(PARTICLE_DRIP_WATER, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
}

void SpongeBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"sponge");
    mWetIcon = iconRegister->registerIcon(L"sponge_wet");
}

TextureAtlasSprite* SpongeBlock::getTexture(const Direction*, const BlockState* state)
{
    return state->getValue(WET) ? mWetIcon : mIcon;
}
