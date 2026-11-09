#include "game/world/block/StructureVoidBlock.hpp"

#include "game/world/block/Material.hpp"

StructureVoidBlock::StructureVoidBlock() : Block(Material::AIR)
{
    DerivedInit();
}

EBlockRenderShape StructureVoidBlock::getRenderShape(const BlockState* state)
{
    return BLOCK_RENDER_INVISIBLE;
}

const AABB* StructureVoidBlock::getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return NULL;
}

bool StructureVoidBlock::isSolidRender(const BlockState* state) const
{
    return false;
}

bool StructureVoidBlock::mayPick(const BlockState* state, bool includeLiquid)
{
    return false;
}

void StructureVoidBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                        int lootBonus)
{
}

bool StructureVoidBlock::mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos)
{
    return true;
}

EPistonPushReaction StructureVoidBlock::getPistonPushReaction(const BlockState* state)
{
    return PISTON_PUSH_DESTROY;
}

EBlockFaceShape StructureVoidBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state,
                                                      const BlockPos& pos, const Direction* direction)
{
    return BLOCK_FACE_UNDEFINED;
}
