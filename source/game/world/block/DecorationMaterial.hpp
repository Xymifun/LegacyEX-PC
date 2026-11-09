#ifndef GAME_WORLD_BLOCK_DECORATIONMATERIAL_HPP
#define GAME_WORLD_BLOCK_DECORATIONMATERIAL_HPP

#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

class DecorationMaterial : public Material
{
public:
    DecorationMaterial(const MaterialColor* color) : Material(color) { makeDestroyedByHand(); }
    virtual ~DecorationMaterial();

    virtual bool isSolid();
    virtual bool blocksLight();
    virtual bool blocksMotion();
};

#endif
