#ifndef GAME_WORLD_BLOCK_MATERIALSHADERS_HPP
#define GAME_WORLD_BLOCK_MATERIALSHADERS_HPP

#include "game/client/render/C4JRender.hpp"

class MaterialShaders
{
public:
    MaterialShaders();

public:
    C4JRender::eVertexType mVertexType;
    C4JRender::ePixelShaderType mPixelShaderType;
};

#endif
