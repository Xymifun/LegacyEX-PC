#ifndef GAME_WORLD_BLOCK_REPEATERBLOCK_HPP
#define GAME_WORLD_BLOCK_REPEATERBLOCK_HPP

#include "game/world/block/DiodeBlock.hpp"

#include <boost/shared_ptr.hpp>

class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class Level;
class LevelSource;
class Mirror;
class Player;
class Random;
class Rotation;
class Item;
template <class T> class AbstractProperty;

class RepeaterBlock : public DiodeBlock
{
public:
    static const double DELAY_TORCH_OFFSETS[4];
    static AbstractProperty<bool>* LOCKED;
    static AbstractProperty<int>* DELAY;

    RepeaterBlock(bool on);
    virtual std::wstring getName();
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                              const BlockPos& pos);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual int getTurnOnDelay(const BlockState* state);
    virtual const BlockState* getOnBlock(const BlockState* state);
    virtual const BlockState* getOffBlock(const BlockState* state);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool isLocked(LevelSource* levelSource, const BlockPos& pos, const BlockState* state);
    virtual bool isAlternateInput(const BlockState* state);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual bool TestUse();
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
};

#endif
