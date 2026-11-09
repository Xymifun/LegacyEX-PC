#ifndef GAME_ENTITY_MOB_BOSSMOB_HPP
#define GAME_ENTITY_MOB_BOSSMOB_HPP

#include <string>

class BossMob
{
public:
    virtual ~BossMob() {}

    virtual float getMaxHealth() = 0;
    virtual float getHealth() = 0;
    virtual std::wstring getDisplayName() = 0;
    virtual int getDimension() = 0;
};

#endif
