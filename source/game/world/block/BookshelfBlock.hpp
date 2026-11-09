#ifndef GAME_WORLD_BLOCK_BOOKSHELFBLOCK_HPP
#define GAME_WORLD_BLOCK_BOOKSHELFBLOCK_HPP

#include "game/world/block/Block.hpp"

class BookshelfBlock : public Block
{
public:
    BookshelfBlock();
    virtual int getResourceCount(Random* random);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
};

#endif
