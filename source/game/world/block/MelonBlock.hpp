#ifndef GAME_WORLD_BLOCK_MELONBLOCK_HPP
#define GAME_WORLD_BLOCK_MELONBLOCK_HPP

#include "game/world/block/Block.hpp"

class MelonBlock : public Block
{
public:
    MelonBlock();
    virtual int getResourceCount(Random* random);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int getResourceCountForLootBonus(int lootBonus, Random* random);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);

protected:
    TextureAtlasSprite* mTopIcon;
};

#endif
