#ifndef GAME_ENTITY_AI_GOAL_HPP
#define GAME_ENTITY_AI_GOAL_HPP

#include <string>

class Level;

class Goal
{
public:
    struct FloatRange
    {
        float min;
        float max;

        FloatRange(float minValue, float maxValue) : min(minValue), max(maxValue) {}
    };

    Goal();
    virtual ~Goal() {}

    virtual bool canUse() = 0;
    virtual bool canContinueToUse();
    virtual bool canInterrupt();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual void setRequiredControlFlags(int flags);
    virtual int getRequiredControlFlags();
    virtual void setLevel(Level* level);
    virtual std::wstring getName() = 0;
    std::wstring toString();

protected:
    int mRequiredControlFlags;
};

#endif
