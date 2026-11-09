#ifndef GAME_WORLD_BLOCK_USEBLOCKTASK_HPP
#define GAME_WORLD_BLOCK_USEBLOCKTASK_HPP

#include <vector>

#include "game/minigame/TutorialTask.hpp"
#include "game/world/block/BlockPos.hpp"

class Block;
class Level;

class UseBlockTask : public TutorialTask
{
public:
    UseBlockTask(Block* block, Tutorial* tutorial, int descriptionId, bool preCompletionEnabled,
                 std::vector<TutorialConstraint*>* constraints, bool showMinimumTime, bool allowFade,
                 bool repeatMessage);
    UseBlockTask(Block* block, const BlockPos& pos, Tutorial* tutorial, int descriptionId, bool preCompletionEnabled,
                 std::vector<TutorialConstraint*>* constraints, bool showMinimumTime, bool allowFade,
                 bool repeatMessage);

    virtual bool isCompleted();
    virtual void useItemOn(Level* level, not_null_ptr<ItemInstance> item, const BlockPos& pos, bool cancelled);

private:
    BlockPos mPos;
    Block* mBlock;
    bool mHasPosition;
};

#endif
