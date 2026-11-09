#include "game/world/block/AirBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

AirBlock::AirBlock() : Block(Material::AIR)
{
    DerivedInit();
}

EBlockRenderShape AirBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_INVISIBLE;
}

const AABB* AirBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL_AABB;
}

bool AirBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool AirBlock::mayPick(const BlockState*, bool)
{
    return false;
}

void AirBlock::spawnResources(Level*, const BlockPos&, const BlockState*, float, int)
{
}

bool AirBlock::mayReplaceWithPlace(LevelSource*, const BlockPos&)
{
    return true;
}

bool AirBlock::isCubeShaped(const BlockState*)
{
    return false;
}

EBlockFaceShape AirBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}
