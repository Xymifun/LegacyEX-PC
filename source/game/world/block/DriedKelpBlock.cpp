#include "game/world/block/DriedKelpBlock.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/core/Direction.hpp"
#include "game/core/FlippedIcon.hpp"
#include "game/core/IconRegister.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

DriedKelpBlock::DriedKelpBlock(Material* material) : Block(material)
{
    DerivedInit();
    setTicking(true);
    memset(mSideIcons, 0, sizeof(mSideIcons));
}

void DriedKelpBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (!level->mIsClientSide && level->getBlockId(pos.x, pos.y + 1, pos.z) == BlockId::FIRE
        && level->isUnderWater(pos, 269, 0))
    {
        level->removeBlock(pos.above());
    }
}

void DriedKelpBlock::tick(Level*, const BlockPos&, const BlockState*, Random*)
{
}

EPistonPushReaction DriedKelpBlock::getPistonPushReaction(const BlockState*)
{
    return PISTON_PUSH_NORMAL;
}

TextureAtlasSprite* DriedKelpBlock::getTexture(const Direction* direction, const BlockState*)
{
    if (direction == Direction::NORTH || direction == Direction::WEST)
    {
        return mSideIcons[0];
    }
    if (direction == Direction::SOUTH || direction == Direction::EAST)
    {
        return mSideIcons[1];
    }
    return mTopIcon;
}

void DriedKelpBlock::registerIcons(IconRegister* iconRegister)
{
    mSideIcons[0] = iconRegister->registerIcon(L"dried_kelp_side");
    mTopIcon = iconRegister->registerIcon(L"dried_kelp_top");
    delete mSideIcons[1];
    mSideIcons[1] = new FlippedIcon(mSideIcons[0], true, false);
}
