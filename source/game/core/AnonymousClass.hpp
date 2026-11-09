#ifndef GAME_CORE_ANONYMOUSCLASS_HPP
#define GAME_CORE_ANONYMOUSCLASS_HPP

#include "game/world/block/BlockInWorld.hpp"

class BlockState;

class AnonymousClass : public Predicate<BlockInWorld*>
{
public:
    virtual bool apply(BlockInWorld* blockInWorld) const;
    virtual ~AnonymousClass();

private:
    const BlockState* mState;
};

#endif
