#ifndef GAME_WORLD_BLOCK_WEBMATERIAL_HPP
#define GAME_WORLD_BLOCK_WEBMATERIAL_HPP

#include "game/world/block/Material.hpp"

class WebMaterial : public Material
{
public:
    WebMaterial(const MaterialColor* color) : Material(color) {}
    virtual ~WebMaterial();

    virtual bool isSolid();
    virtual bool blocksMotion();
};

#endif
