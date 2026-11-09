#ifndef GAME_ENTITY_AI_TEMPTGOAL_HPP
#define GAME_ENTITY_AI_TEMPTGOAL_HPP

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/entity/ai/Goal.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class Item;
class ItemInstance;
class Player;
class PathfinderMob;

class TemptGoal : public Goal
{
public:
    TemptGoal(PathfinderMob* mob, double speedModifier, arrayWithLength<Item*> items, bool canScare,
              arrayWithLength<int> itemAuxValues);

    bool shouldFollowItem(not_null_ptr<ItemInstance> item);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    bool isRunning();
    virtual std::wstring getName() { return L"Tempt"; }

private:
    PathfinderMob* mMob;
    double mSpeedModifier;
    double mTargetX;
    double mTargetY;
    double mTargetZ;
    double mTargetXRot;
    double mTargetYRot;
    boost::weak_ptr<Player> mPlayer;
    int mDelayTemptCounter;
    bool mRunning;
    arrayWithLength<Item*> mItems;
    arrayWithLength<int> mItemAuxValues;
    bool mCanScare;
    bool mField0x61;
};

#endif
