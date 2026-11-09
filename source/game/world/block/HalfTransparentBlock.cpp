#include "game/world/block/HalfTransparentBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

HalfTransparentBlock::HalfTransparentBlock(const std::wstring& iconName, Material* material, bool ignoreSimilarity)
    : Block(material, material->getColor())
{
    mIgnoreSimilarity = ignoreSimilarity;
    mTextureName = iconName;
}

HalfTransparentBlock::HalfTransparentBlock(const std::wstring& iconName, Material* material, bool ignoreSimilarity,
                                           const MaterialColor* materialColor)
    : Block(material, materialColor)
{
    mIgnoreSimilarity = ignoreSimilarity;
    mTextureName = iconName;
}

bool HalfTransparentBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                            const Direction* direction)
{
    const BlockState* neighbourState = levelSource->getBlockState(pos.relative(direction));
    Block* neighbourBlock = neighbourState->getBlock();

    if (this == Blocks::glass || this == Blocks::stained_glass)
    {
        if (state != neighbourState)
        {
            return true;
        }

        if (neighbourBlock == this)
        {
            return false;
        }
    }

    if (!mIgnoreSimilarity && neighbourBlock == this)
    {
        return false;
    }

    return Block::shouldRenderFace(state, levelSource, pos, direction);
}

bool HalfTransparentBlock::isSolidRender(const BlockState* state) const
{
    return false;
}

void HalfTransparentBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(mTextureName);
}
