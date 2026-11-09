#ifndef GAME_ITEM_LOCKABLECONTAINER_HPP
#define GAME_ITEM_LOCKABLECONTAINER_HPP

#include "game/core/InteractionObject.hpp"
#include "game/item/Container.hpp"

class LockCode;

class LockableContainer : public virtual Container, public InteractionObject
{
public:
    LockableContainer();

    virtual bool isLocked() = 0;
    virtual void setLocked(LockCode* lockCode) = 0;
    virtual LockCode* getLockKeyCode() = 0;
    virtual void unlock() = 0;
    virtual ~LockableContainer();
};

#endif
