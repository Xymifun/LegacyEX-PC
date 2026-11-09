#include "game/world/block/RotatedPillarBlockImpl.hpp"

#include "game/core/Boxed.hpp"
#include "game/core/IconRegister.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

RotatedPillarBlockImpl::RotatedPillarBlockImpl(Material* material) : RotatedPillarBlock(material, material->getColor())
{
    DerivedInit();
}

RotatedPillarBlockImpl::RotatedPillarBlockImpl(Material* material, const MaterialColor* materialColor)
    : RotatedPillarBlock(material, materialColor)
{
    DerivedInit();
}

void RotatedPillarBlockImpl::DerivedInit()
{
    Block::DerivedInit();
    const BlockState* state = mStateDefinition->any();
    registerDefaultState(state->setValue(gRotatedPillarBlockAxisProperty, Direction::Axis::Y));
}

Direction::Axis* RotatedPillarBlockImpl::getAxis(const BlockState* state)
{
    return state->getValue(gRotatedPillarBlockAxisProperty);
}

RotatedPillarBlockImpl* RotatedPillarBlockImpl::setIconTopName(const std::wstring& name)
{
    mIconTopName.assign(name, 0, std::wstring::npos);
    return this;
}

void RotatedPillarBlockImpl::registerIcons(IconRegister* iconRegister)
{
    mTopTexture = iconRegister->registerIcon(mIconTopName);
    mIcon = iconRegister->registerIcon(mIconName);
}
