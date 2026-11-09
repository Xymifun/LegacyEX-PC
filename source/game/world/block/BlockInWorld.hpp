#ifndef GAME_WORLD_BLOCK_BLOCKINWORLD_HPP
#define GAME_WORLD_BLOCK_BLOCKINWORLD_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Predicate.hpp"

class BlockEntity;
class BlockState;
class Level;

class BlockInWorld
{
public:
    class BlockInWorldPredicate : public Predicate<BlockInWorld*>
    {
    public:
        BlockInWorldPredicate(const Predicate<const BlockState*>* predicate);
        virtual bool apply(BlockInWorld* blockInWorld) const;
        virtual ~BlockInWorldPredicate();

    private:
        const Predicate<const BlockState*>* mPredicate;
    };

    BlockInWorld(Level* level, const BlockPos& pos, bool loadChunks);

    const BlockState* getState();
    boost::shared_ptr<BlockEntity> getEntity();
    BlockPos getPos();
    Level* getLevel();

    static Predicate<BlockInWorld*>* hasState(const Predicate<const BlockState*>* predicate);
    static Predicate<BlockInWorld*>* hasState(const BlockState* state);

private:
    Level* mLevel;
    BlockPos mPos;
    bool mLoadChunks;
    const BlockState* mState;
    boost::shared_ptr<BlockEntity> mEntity;
    bool mEntityCached;
};

#endif
