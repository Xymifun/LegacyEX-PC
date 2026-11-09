#include "game/world/block/StructureBlock.hpp"

#include "game/world/block/Material.hpp"

StructureBlock::StructureBlock() : Block(Material::AIR)
{
    DerivedInit();
}

EBlockRenderShape StructureBlock::getRenderShape(const BlockState* state)
{
    return BLOCK_RENDER_INVISIBLE;
}

const AABB* StructureBlock::getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return NULL;
}

bool StructureBlock::isSolidRender(const BlockState* state) const
{
    return false;
}

bool StructureBlock::mayPick(const BlockState* state, bool includeLiquid)
{
    return false;
}

void StructureBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                    int lootBonus)
{
}

bool StructureBlock::mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos)
{
    return true;
}
