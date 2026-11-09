#include "game/world/block/BoneBlock.hpp"

#include "game/client/audio/SoundType.hpp"
#include "game/core/IconRegister.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

BoneBlock::BoneBlock() : RotatedPillarBlock(Material::STONE, MaterialColor::SAND)
{
    DerivedInit();

    registerDefaultState(mStateDefinition->any()->setValue(gRotatedPillarBlockAxisProperty, Direction::Axis::Y));
    setDestroyTime(2.0f);
    setSoundType(SoundType::STONE);
}

TextureAtlasSprite* BoneBlock::getTypeTexture(const BlockState*)
{
    return mIcon;
}

void BoneBlock::registerIcons(IconRegister* iconRegister)
{
    mTopTexture = iconRegister->registerIcon(getIconName() + L"_top");
    mIcon = iconRegister->registerIcon(getIconName() + L"_side");
}

EBlockRenderShape BoneBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_TREE;
}

Direction::Axis* BoneBlock::getAxis(const BlockState* state)
{
    return state->getValue(gRotatedPillarBlockAxisProperty);
}
