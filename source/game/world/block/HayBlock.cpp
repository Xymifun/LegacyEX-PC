#include "game/world/block/HayBlock.hpp"

#include "game/core/IconRegister.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

HayBlock::HayBlock() : RotatedPillarBlock(Material::GRASS, MaterialColor::COLOR_YELLOW)
{
    DerivedInit();

    registerDefaultState(mStateDefinition->any()->setValue(gRotatedPillarBlockAxisProperty, Direction::Axis::Y));
}

void HayBlock::fallOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, float fallDistance)
{
    entity->causeFallDamage(fallDistance, 0.2f);
}

TextureAtlasSprite* HayBlock::getTypeTexture(const BlockState*)
{
    return mIcon;
}

void HayBlock::registerIcons(IconRegister* iconRegister)
{
    mTopTexture = iconRegister->registerIcon(getIconName() + L"_top");
    mIcon = iconRegister->registerIcon(getIconName() + L"_side");
}

EBlockRenderShape HayBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_TREE;
}

Direction::Axis* HayBlock::getAxis(const BlockState* state)
{
    return state->getValue(gRotatedPillarBlockAxisProperty);
}
