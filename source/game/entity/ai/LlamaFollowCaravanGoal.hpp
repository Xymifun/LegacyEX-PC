#ifndef GAME_ENTITY_AI_LLAMAFOLLOWCARAVANGOAL_HPP
#define GAME_ENTITY_AI_LLAMAFOLLOWCARAVANGOAL_HPP

#include <boost/shared_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"

class Entity;
class Llama;

class LlamaFollowCaravanGoal : public Goal
{
public:
    LlamaFollowCaravanGoal(Llama* llama, double speedModifier);
    virtual ~LlamaFollowCaravanGoal();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName();

private:
    bool firstIsLeashed(Llama* llama, int& distance, boost::shared_ptr<Entity>& leashHolder);

    Llama* mLlama;
    double mSpeedModifier;
    int mDistanceToStop;
};

#endif
