#ifndef GAME_WORLD_BLOCK_NOTEBLOCK_HPP
#define GAME_WORLD_BLOCK_NOTEBLOCK_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/BaseEntityBlock.hpp"
#include "game/world/block/EntityBlock.hpp"

class BlockEntity;
class BlockPos;
class BlockState;
class BlockTooltipDataHolder;
class Direction;
class Level;
class Player;
class SoundEvent;

class NoteBlock : public BaseEntityBlock
{
public:
    NoteBlock();

    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual bool TestUse();
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual void attack(Level* level, const BlockPos& pos, boost::shared_ptr<Player> player);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual unsigned int GetHitTooltip(const BlockTooltipDataHolder& tooltipData);
    const SoundEvent* getInstrument(int instrument);
    virtual bool triggerEvent(const BlockState* state, Level* level, const BlockPos& pos, int eventId, int eventParam);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);

    static void blockStaticCtor();
};

#endif
