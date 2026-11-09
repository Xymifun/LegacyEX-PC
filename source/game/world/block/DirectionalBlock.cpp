#include "game/world/block/DirectionalBlock.hpp"

#include "game/world/block/Material.hpp"

DirectionProperty* DirectionalBlock::FACING = NULL;
DirectionProperty* DirectionalBlock::FACING_PROPERTY = NULL;

void DirectionalBlock::blockStaticCtor()
{
    FACING_PROPERTY = FACING;
}

DirectionalBlock::DirectionalBlock(Material* material) : Block(material)
{
}

DirectionalBlock::DirectionalBlock(Material* material, const MaterialColor* materialColor)
    : Block(material, materialColor)
{
}
