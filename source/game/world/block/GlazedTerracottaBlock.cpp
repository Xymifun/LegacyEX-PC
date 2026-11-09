#include "game/world/block/GlazedTerracottaBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/world/block/TypedBoxed.hpp"

GlazedTerracottaBlock::GlazedTerracottaBlock(const DyeColor* dyeColor)
    : HorizontalDirectionalBlock(Material::STONE, MaterialColor::getDyeMaterial(dyeColor))
{
    DerivedInit();
    setDestroyTime(1.4f);
    setSoundType(SoundType::STONE);
}

const BlockState* GlazedTerracottaBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(
        HorizontalDirectionalBlock::FACING_PROPERTY,
        rotation->rotate(state->getTypedBoxed(HorizontalDirectionalBlock::FACING_PROPERTY)->getValue()));
}

const BlockState* GlazedTerracottaBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return state->rotate(mirror->getRotation(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)));
}

const BlockState* GlazedTerracottaBlock::getStateForPlacement(Level*, const BlockPos&, const Direction*, float, float,
                                                              float, int, boost::shared_ptr<LivingEntity> placer)
{
    const BlockState* state = defaultBlockState();
    DirectionProperty* facingProperty = HorizontalDirectionalBlock::FACING_PROPERTY;
    const Direction* facing = placer->getDirection()->getOpposite();
    TypedBoxed<const Direction*> boxedFacing(facing);
    return state->setBoxedValue(facingProperty, &boxedFacing);
}

int GlazedTerracottaBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    DirectionProperty* facingProperty = HorizontalDirectionalBlock::FACING_PROPERTY;
    return state->getValue(facingProperty)->get2DDataValue();
}

const BlockState* GlazedTerracottaBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState();
    DirectionProperty* facingProperty = HorizontalDirectionalBlock::FACING_PROPERTY;
    TypedBoxed<const Direction*> facing(Direction::from2DDataValue(data));
    return state->setBoxedValue(facingProperty, &facing);
}

EPistonPushReaction GlazedTerracottaBlock::getPistonPushReaction(const BlockState*)
{
    return PISTON_PUSH_PUSH_ONLY;
}

EBlockRenderShape GlazedTerracottaBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_HORIZONTAL_DIRECTIONAL;
}

BlockStateDefinition* GlazedTerracottaBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {HorizontalDirectionalBlock::FACING_PROPERTY};
    return new BlockStateDefinition(this, properties, 1);
}
