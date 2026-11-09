#ifndef GAME_WORLD_BLOCK_HAYBLOCK_HPP
#define GAME_WORLD_BLOCK_HAYBLOCK_HPP

#include "game/core/Direction.hpp"
#include "game/world/block/RotatedPillarBlock.hpp"

class BlockState;
class IconRegister;
class Level;
class Entity;

class HayBlock : public RotatedPillarBlock
{
public:
    HayBlock();

    virtual void fallOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, float fallDistance);
    virtual TextureAtlasSprite* getTypeTexture(const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual Direction::Axis* getAxis(const BlockState* state);
};

#endif
