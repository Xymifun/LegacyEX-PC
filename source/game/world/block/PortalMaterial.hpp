#ifndef GAME_WORLD_BLOCK_PORTALMATERIAL_HPP
#define GAME_WORLD_BLOCK_PORTALMATERIAL_HPP

#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

class PortalMaterial : public Material
{
public:
    PortalMaterial(const MaterialColor* color) : Material(color) {}
    virtual ~PortalMaterial();

    virtual bool isSolid();
    virtual bool blocksLight();
    virtual bool blocksMotion();
};

#endif
