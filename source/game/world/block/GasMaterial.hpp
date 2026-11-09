#ifndef GAME_WORLD_BLOCK_GASMATERIAL_HPP
#define GAME_WORLD_BLOCK_GASMATERIAL_HPP

#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

class GasMaterial : public Material
{
public:
    GasMaterial(const MaterialColor* color) : Material(color) { replaceable(); }
    virtual ~GasMaterial();

    virtual bool isSolid();
    virtual bool blocksLight();
    virtual bool blocksMotion();
};

#endif
