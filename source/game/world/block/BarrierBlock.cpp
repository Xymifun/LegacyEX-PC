#include "game/world/block/BarrierBlock.hpp"

#include "game/world/block/Material.hpp"

BarrierBlock::BarrierBlock() : Block(Material::BARRIER)
{
    DerivedInit();
    setIndestructible();
    setExplodeable(6000001.0f);
    setNotCollectStatistics();
    mTranslucent = true;
}

EBlockRenderShape BarrierBlock::getRenderShape(const BlockState* state)
{
    return BLOCK_RENDER_INVISIBLE;
}

bool BarrierBlock::isSolidRender(const BlockState* state) const
{
    return false;
}

float BarrierBlock::getShadeBrightness(const BlockState* state)
{
    return 1.0f;
}

void BarrierBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                  int lootBonus)
{
}
