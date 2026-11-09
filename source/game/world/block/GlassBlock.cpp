#include "game/world/block/GlassBlock.hpp"

#include "game/util/Random.hpp"

GlassBlock::GlassBlock(Material* material, bool ignoreSimilarity)
    : HalfTransparentBlock(L"glass", material, ignoreSimilarity)
{
    DerivedInit();
}

int GlassBlock::getResourceCount(Random*)
{
    return 0;
}

int GlassBlock::getRenderLayer()
{
    return 0;
}

bool GlassBlock::isCubeShaped(const BlockState*)
{
    return true;
}

bool GlassBlock::isTransparent(const BlockState*)
{
    return true;
}

bool GlassBlock::isSilkTouchable()
{
    return true;
}

bool GlassBlock::canContainLiquid(const BlockState*) const
{
    return false;
}
