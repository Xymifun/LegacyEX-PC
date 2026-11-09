#ifndef GAME_WORLD_BLOCK_LIQUIDMATERIAL_HPP
#define GAME_WORLD_BLOCK_LIQUIDMATERIAL_HPP

#include "game/world/block/Material.hpp"

class MaterialColor;

class LiquidMaterial : public Material
{
public:
    LiquidMaterial(const MaterialColor* color) : Material(color) { replaceable(); }
    virtual ~LiquidMaterial();

    virtual bool isLiquid();
    virtual bool isSolid();
    virtual bool blocksMotion();
};

#endif
