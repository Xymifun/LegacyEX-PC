#include "game/world/block/TransparentBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

TransparentBlock::TransparentBlock(Material* material, bool ignoreSimilarity) : Block(material)
{
    mIgnoreSimilarity = ignoreSimilarity;
}

bool TransparentBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                        const Direction* direction)
{
    if (!mIgnoreSimilarity && levelSource->getBlockState(pos)->getBlock() == this)
    {
        return false;
    }

    return defaultBlockState()->shouldRenderFace(levelSource, pos, direction);
}

bool TransparentBlock::isSolidRender(const BlockState* state) const
{
    return false;
}
