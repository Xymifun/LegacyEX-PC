#ifndef GAME_WORLD_BLOCK_BONEBLOCK_HPP
#define GAME_WORLD_BLOCK_BONEBLOCK_HPP

#include "game/core/Direction.hpp"
#include "game/world/block/RotatedPillarBlock.hpp"

class BlockState;
class IconRegister;

class BoneBlock : public RotatedPillarBlock
{
public:
    BoneBlock();

    virtual TextureAtlasSprite* getTypeTexture(const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual Direction::Axis* getAxis(const BlockState* state);
};

#endif
