#include "game/world/block/BlockImpl.hpp"

#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

BlockImpl::BlockImpl(Material* material) : Block(material)
{
    Block::DerivedInit();
}

BlockImpl::BlockImpl(Material* material, const MaterialColor* materialColor) : Block(material, materialColor)
{
    Block::DerivedInit();
}
