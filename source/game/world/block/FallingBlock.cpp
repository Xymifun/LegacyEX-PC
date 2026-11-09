#include "game/world/block/FallingBlock.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/FallingBlockEntity.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/biome/arrayWithLength.hpp"

bool FallingBlock::fallInstantly;

FallingBlock::FallingBlock() : Block(Material::SAND)
{
    mBlockType = EBLOCK_TYPE_FALLING;
}

FallingBlock::FallingBlock(Material* material) : Block(material)
{
    mBlockType = EBLOCK_TYPE_FALLING;
}

void FallingBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    level->addToTickNextTick(pos, this, getTickDelay(level));
}

void FallingBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                   const BlockPos& fromPos)
{
    level->addToTickNextTick(pos, this, getTickDelay(level));
}

bool FallingBlock::isFree(const BlockState* state)
{
    Block* block = state->getBlock();
    Material* material = state->getMaterial();
    return block == Blocks::fire || material == Material::AIR || material->isLiquid();
}

void FallingBlock::checkSlide(Level* level, const BlockPos& pos)
{
    if (isFree(level->getBlockState(pos.below())) && pos.y >= 0)
    {
        if (fallInstantly || !level->hasChunksAt(pos.offset(-32, -32, -32), pos.offset(32, 32, 32)))
        {
            level->removeBlock(pos);
            BlockPos target = pos.below();
            while (isFree(level->getBlockState(target)) && target.y > 0)
            {
                target = target.below();
            }

            if (target.y > 0)
            {
                level->setBlockAndUpdate(target.above(), defaultBlockState());
            }
        }
        else if (!level->mIsClientSide)
        {
            if (!level->newFallingBlockAllowed())
            {
                level->forceAddBlockTick(pos, this, getTickDelay(level), 0);
                return;
            }

            boost::shared_ptr<FallingBlockEntity> entity(new FallingBlockEntity(
                level, (double)pos.x + 0.5, (double)pos.y, (double)pos.z + 0.5, level->getBlockState(pos)));
            falling(entity);
            level->addEntity(entity);
        }
    }
}

void FallingBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (!level->mIsClientSide)
    {
        checkSlide(level, pos);
    }
}

void FallingBlock::falling(boost::shared_ptr<FallingBlockEntity> entity)
{
}

int FallingBlock::getTickDelay(Level* level)
{
    return 2;
}

void FallingBlock::onLand(Level* level, const BlockPos& pos, const BlockState* state, const BlockState* replacedState)
{
}

void FallingBlock::onBroken(Level* level, const BlockPos& pos)
{
}

void FallingBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    if (random->nextInt(16) == 0)
    {
        BlockPos below = pos.below();
        if (isFree(level->getBlockState(below)))
        {
            double x = pos.x + random->nextFloat();
            double y = pos.y - 0.05;
            double z = pos.z + random->nextFloat();
            arrayWithLength<int> params(1, true);
            params.data[0] = Block::getIdWithData(state);
            level->addParticle(PARTICLE_FALLING_DUST, x, y, z, 0.0, 0.0, 0.0, params);
            delete params.data;
        }
    }
}

unsigned int FallingBlock::getDustColor(const BlockState* state)
{
    return 0xff000000;
}
