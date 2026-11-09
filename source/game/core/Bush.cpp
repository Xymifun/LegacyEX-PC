#include "game/core/Bush.hpp"
#include "game/world/LevelEvent.hpp"

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

static const AABB* BUSH_SHAPE = AABB::newPermanent(0.3, 0.0, 0.3, 0.7, 0.6, 0.7);

Bush::Bush() : Block(Material::PLANT)
{
    _init();
}

Bush::Bush(Material* material) : Block(material)
{
    _init();
}

Bush::Bush(Material* material, const MaterialColor* materialColor) : Block(material, materialColor)
{
    _init();
}

void Bush::_init()
{
    setTicking(true);
}

bool Bush::mayPlace(Level* level, const BlockPos& pos)
{
    return Block::mayPlace(level, pos) && mayPlaceOn(level, pos.below(), level->getBlockState(pos.below()));
}

bool Bush::mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state)
{
    return state->getBlock() == Blocks::grass || state->getBlock() == Blocks::dirt
           || state->getBlock() == Blocks::farmland;
}

void Bush::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                           const BlockPos& fromPos)
{
    if (!canSurvive(level, pos, state))
    {
        level->addToTickNextTick(pos, state->getBlock(), 1);
    }
}

void Bush::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    checkAlive(level, pos, state);
}

void Bush::checkAlive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!canSurvive(level, pos, state))
    {
        Block* block = defaultBlockState()->getBlock();
        if (block == Blocks::tallgrass || block == Blocks::double_plant || block == Blocks::deadbush)
        {
            level->levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, pos, Block::getIdWithData(state));
        }

        spawnResources(level, pos, state, 0);
        level->setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);
    }
}

bool Bush::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    return mayPlaceOn(level, pos.below(), level->getBlockState(pos.below()));
}

const AABB* Bush::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return BUSH_SHAPE;
}

const AABB* Bush::getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return NULL;
}

bool Bush::waterSpreadCausesSpawn()
{
    return true;
}

bool Bush::isSolidRender(const BlockState* state) const
{
    return false;
}

bool Bush::isCubeShaped(const BlockState* state)
{
    return false;
}

int Bush::getRenderLayer()
{
    return 0;
}

EBlockFaceShape Bush::getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                        const Direction* direction)
{
    return BLOCK_FACE_UNDEFINED;
}

EBlockRenderShape Bush::getRenderShape(const BlockState* state)
{
    return BLOCK_RENDER_CROSS;
}
