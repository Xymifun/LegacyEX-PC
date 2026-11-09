#ifndef GAME_CORE_GIANT_HPP
#define GAME_CORE_GIANT_HPP

#include "game/entity/mob/Monster.hpp"

class BlockPos;
class DataFixerUpper;
class Level;

class Giant : public Monster
{
public:
    Giant(Level* level);

    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual float getEyeHeight();
    virtual void registerAttributes();
    virtual float getWalkTargetValue(const BlockPos& pos);
    virtual ResourceLocation* getDefaultLootTable();
    virtual unsigned long long GetType();
};

#endif
