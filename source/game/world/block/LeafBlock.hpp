#ifndef GAME_WORLD_BLOCK_LEAFBLOCK_HPP
#define GAME_WORLD_BLOCK_LEAFBLOCK_HPP

#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TransparentBlock.hpp"

class BlockPos;
class BlockState;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class Random;
class TextureAtlasSprite;

template <class T> class AbstractProperty;

class LeafBlock : public TransparentBlock
{
public:
    static AbstractProperty<bool>* DECAYABLE;
    static AbstractProperty<bool>* CHECK_DECAY;

    LeafBlock();
    virtual ~LeafBlock();

    static bool isLeaves(Block* block);

    virtual void DerivedInit();
    virtual int getColor() const;
    virtual int getColor(const BlockState* state);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, int tintIndex);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    void die(Level* level, const BlockPos& pos);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual int getResourceCount(Random* random);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
    virtual void dropExtraLoot(Level* level, const BlockPos& pos, const BlockState* state, int lootBonus);
    virtual int getLootDropChance(const BlockState* state);
    virtual bool isSolidRender(const BlockState* state) const;
    void setFancy(bool fancy);
    virtual int getRenderLayer();
    virtual bool isViewBlocking(const BlockState* state);
    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    virtual bool shouldBlockTick(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getDescriptionId(int auxValue);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual bool canProvideSupport(const BlockState* state, const Direction* direction, EBlockSupportType supportType);
    virtual PlanksBlock::Variant* getWoodVariantForData(int data) = 0;
    virtual int getWoodType(const BlockState* state) = 0;

private:
    int* mLeafDistance;
    TextureAtlasSprite* mIcons[12];
};

#endif
