#ifndef GAME_WORLD_BLOCK_GLASSBLOCK_HPP
#define GAME_WORLD_BLOCK_GLASSBLOCK_HPP

#include "game/world/block/HalfTransparentBlock.hpp"

class BlockState;
class Material;
class Random;

class GlassBlock : public HalfTransparentBlock
{
public:
    GlassBlock(Material* material, bool ignoreSimilarity);
    virtual int getResourceCount(Random* random);
    virtual int getRenderLayer();
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isTransparent(const BlockState* state);
    virtual bool isSilkTouchable();
    virtual bool canContainLiquid(const BlockState* state) const;
};

#endif
