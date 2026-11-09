#ifndef GAME_WORLD_BLOCK_TRANSPARENTBLOCK_HPP
#define GAME_WORLD_BLOCK_TRANSPARENTBLOCK_HPP

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class Direction;
class LevelSource;
class Material;

class TransparentBlock : public Block
{
public:
    TransparentBlock(Material* material, bool ignoreSimilarity);

    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    virtual bool isSolidRender(const BlockState* state) const;

protected:
    bool mIgnoreSimilarity;
};

#endif
