#ifndef GAME_CORE_HOPPER_HPP
#define GAME_CORE_HOPPER_HPP

#include "game/item/Container.hpp"

class Level;

class Hopper : public virtual Container
{
public:
    Hopper();

    virtual Level* getLevel() = 0;
    virtual double getLevelX() = 0;
    virtual double getLevelY() = 0;
    virtual double getLevelZ() = 0;

    virtual ~Hopper();
};

#endif
