#ifndef GAME_WORLD_BLOCK_OCELOTSITONBLOCKGOAL_HPP
#define GAME_WORLD_BLOCK_OCELOTSITONBLOCKGOAL_HPP

#include <string>

#include "game/world/block/MoveToBlockGoal.hpp"

class BlockPos;
class Level;
class Ocelot;

class OcelotSitOnBlockGoal : public MoveToBlockGoal
{
public:
    OcelotSitOnBlockGoal(Ocelot* ocelot, double speedModifier);
    virtual ~OcelotSitOnBlockGoal();

    virtual bool canUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual bool isValidTarget(Level* level, const BlockPos& pos);
    virtual std::wstring getName() { return L"OcelotSitOnBlock"; }

private:
    Ocelot* mOcelot;
};

#endif
