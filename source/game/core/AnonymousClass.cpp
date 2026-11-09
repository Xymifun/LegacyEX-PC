#include "game/core/AnonymousClass.hpp"

bool AnonymousClass::apply(BlockInWorld* blockInWorld) const
{
    return blockInWorld != NULL && blockInWorld->getState() == mState;
}

AnonymousClass::~AnonymousClass()
{
}
