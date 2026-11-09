#include "game/world/block/DragonEggBlock.hpp"
#include "game/world/LevelEvent.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/util/Random.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/FallingBlock.hpp"
#include "game/world/block/FallingBlockEntity.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/biome/arrayWithLength.hpp"

static const AABB* gDragonEggShape = AABB::newPermanent(0.0625, 0.0, 0.0625, 0.9375, 1.0, 0.9375);

DragonEggBlock::DragonEggBlock() : Block(Material::EGG, MaterialColor::COLOR_BLACK)
{
    DerivedInit();
}

const AABB* DragonEggBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return gDragonEggShape;
}

void DragonEggBlock::onPlace(Level* level, const BlockPos& pos, const BlockState*)
{
    level->addToTickNextTick(pos, this, getTickDelay(level));
}

void DragonEggBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    level->addToTickNextTick(pos, this, getTickDelay(level));
}

void DragonEggBlock::checkSlide(Level* level, const BlockPos& pos)
{
    if (!FallingBlock::isFree(level->getBlockState(pos.below())) || pos.y < 0)
        return;
    if (!FallingBlock::fallInstantly && level->hasChunksAt(pos.offset(-32, -32, -32), pos.offset(32, 32, 32)))
    {
        boost::shared_ptr<FallingBlockEntity> entity(
            new FallingBlockEntity(level, pos.x + 0.5f, pos.y, pos.z + 0.5f, defaultBlockState()));
        level->addEntity(entity);
        return;
    }
    level->removeBlock(pos);
    BlockPos target = pos;
    for (; FallingBlock::isFree(level->getBlockState(target)) && target.y > 0; target = target.below())
    {
    }
    if (target.y > 0)
        level->setBlock(target, defaultBlockState(), Block::UPDATE_CLIENTS, false);
}

void DragonEggBlock::tick(Level* level, const BlockPos& pos, const BlockState*, Random*)
{
    checkSlide(level, pos);
}

void DragonEggBlock::teleport(Level* level, const BlockPos& pos)
{
    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() != this)
        return;
    for (int attempt = 0; attempt < 1000; ++attempt)
    {
        BlockPos target = pos.offset(level->mRandom->nextInt(16) - level->mRandom->nextInt(16),
                                     level->mRandom->nextInt(8) - level->mRandom->nextInt(8),
                                     level->mRandom->nextInt(16) - level->mRandom->nextInt(16));
        if (level->getBlockState(target)->getBlock()->mMaterial == Material::AIR)
        {
            if (level->mIsClientSide)
                return;
            level->setBlock(target, state, Block::UPDATE_CLIENTS, false);
            level->removeBlock(pos);
            level->levelEvent(LevelEvent::DRAGON_EGG_TELEPORT, target,
                              ((pos.x - target.x) & 0xff) | (unsigned char)(char)(pos.y - target.y) << 8
                                  | ((pos.z - target.z) << 16 & 0xff0000));
            return;
        }
    }
}

bool DragonEggBlock::use(Level* level, const BlockPos& pos, const BlockState*, boost::shared_ptr<Player>,
                         InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (clientSide)
        return false;
    teleport(level, pos);
    return true;
}

void DragonEggBlock::attack(Level* level, const BlockPos& pos, boost::shared_ptr<Player>)
{
    teleport(level, pos);
}

int DragonEggBlock::getTickDelay(Level*)
{
    return 5;
}

bool DragonEggBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool DragonEggBlock::isCubeShaped(const BlockState*)
{
    return false;
}
bool DragonEggBlock::shouldRenderFace(const BlockState*, LevelSource*, const BlockPos&, const Direction*)
{
    return true;
}
EBlockFaceShape DragonEggBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}
EBlockRenderShape DragonEggBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_EGG;
}

void DragonEggBlock::generateTeleportParticles(Level* level, int x, int y, int z, int data)
{
    const signed char dx = (signed char)(data & 0xff);
    const signed char dy = (signed char)((data >> 8) & 0xff);
    const signed char dz = (signed char)((data >> 16) & 0xff);
    for (int i = 0; i < 64; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            double along = level->mRandom->nextDouble();
            float xa = (level->mRandom->nextFloat() - 0.5f) * 0.2f;
            float ya = (level->mRandom->nextFloat() - 0.5f) * 0.2f;
            float za = (level->mRandom->nextFloat() - 0.5f) * 0.2f;
            double px = (double)x + (double)dx * along + (level->mRandom->nextDouble() - 0.5) * 1.0 + 0.5;
            double py = (double)y + (double)dy * along + level->mRandom->nextDouble() * 1.0 - 0.5;
            double pz = (double)z + (double)dz * along + (level->mRandom->nextDouble() - 0.5) * 1.0 + 0.5;
            arrayWithLength<int> params;
            level->addParticle(PARTICLE_PORTAL, px, py, pz, xa, ya, za, params);
        }
    }
}
