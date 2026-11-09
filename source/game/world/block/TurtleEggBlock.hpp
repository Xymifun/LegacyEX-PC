#ifndef GAME_WORLD_BLOCK_TURTLEEGGBLOCK_HPP
#define GAME_WORLD_BLOCK_TURTLEEGGBLOCK_HPP

#include "game/world/block/Block.hpp"

#include <boost/shared_ptr.hpp>
#include <vector>

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class Entity;
class IconRegister;
class Level;
class LevelSource;
class Player;
class Random;
class TextureAtlasSprite;

template <class T> class AbstractProperty;

class TurtleEggBlock : public Block
{
public:
    TurtleEggBlock();

    bool shouldUpdateHatchLevel(Level* level);
    void _decreaseEggs(Level* level, const BlockPos& pos, int eggCount, bool crack, bool jump) const;
    void _destroyEgg(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, int chance, bool crack,
                     bool jump) const;
    virtual void stepOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity);
    virtual void fallOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, float fallDistance);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual bool playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                   boost::shared_ptr<Player> player);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual bool TestUse(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual void addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                   std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity,
                                   bool isActualState);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual int getRenderLayer();
    virtual bool canInstantlyTick();
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual bool waterSpreadCausesSpawn();
    virtual EPistonPushReaction getPistonPushReaction(const BlockState* state);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual bool isSilkTouchable();

    static AbstractProperty<int>* EGG_COUNT;
    static AbstractProperty<int>* HATCH_LEVEL;
};

#endif
