#include "game/entity/ai/GoalSelector.hpp"

#include <algorithm>

#include "game/core/PIXNamedEvents.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/world/Level.hpp"

GoalSelector::GoalSelector() : mTickCount(0), mNewGoalRate(3), mDisabledControlFlags(0)
{
}

GoalSelector::~GoalSelector()
{
    std::vector<InternalGoal*>::iterator it;
    for (it = mAvailableGoals.begin(); it != mAvailableGoals.end(); it++)
    {
        delete *it;
    }
    mAvailableGoals.clear();
}

GoalSelector::InternalGoal::InternalGoal(int priority, Goal* goal, bool ownsGoal)
{
    mPriority = priority;
    mGoal = goal;
    mOwnsGoal = ownsGoal;
    mIsRunning = false;
}

GoalSelector::InternalGoal::~InternalGoal()
{
    if (mOwnsGoal && mGoal != 0)
    {
        delete mGoal;
    }
}

void GoalSelector::addGoal(int priority, Goal* goal, bool ownsGoal)
{
    mAvailableGoals.push_back(new InternalGoal(priority, goal, ownsGoal));
}

void GoalSelector::removeGoal(Goal* goal)
{
    std::vector<InternalGoal*>::iterator available = mAvailableGoals.begin();
    while (available != mAvailableGoals.end())
    {
        InternalGoal* internalGoal = *available;
        Goal* current = internalGoal->mGoal;
        if (current == goal)
        {
            if (internalGoal->mIsRunning)
            {
                internalGoal->mIsRunning = false;
                current->stop();
                std::vector<InternalGoal*>::iterator running = std::find(mGoals.begin(), mGoals.end(), internalGoal);
                mGoals.erase(running);
            }
            mAvailableGoals.erase(available);
            delete internalGoal;
            return;
        }
        ++available;
    }
}

bool GoalSelector::isControlFlagDisabled(int controlFlag)
{
    return (mDisabledControlFlags & controlFlag) > 0;
}

bool GoalSelector::canCoExist(InternalGoal* goal, InternalGoal* otherGoal)
{
    return (goal->mGoal->getRequiredControlFlags() & otherGoal->mGoal->getRequiredControlFlags()) == 0;
}

bool GoalSelector::canUseInSystem(InternalGoal* goal)
{
    if (isControlFlagDisabled(goal->mGoal->getRequiredControlFlags()))
    {
        return false;
    }
    if (!mGoals.empty())
    {
        for (std::vector<InternalGoal*>::iterator it = mGoals.begin(); it != mGoals.end(); ++it)
        {
            InternalGoal* runningGoal = *it;
            if (runningGoal != goal)
            {
                if (goal->mPriority >= runningGoal->mPriority)
                {
                    if (!canCoExist(goal, runningGoal))
                    {
                        return false;
                    }
                }
                else if (!runningGoal->mGoal->canInterrupt())
                {
                    return false;
                }
            }
        }
    }
    return true;
}

bool GoalSelector::canContinueToUse(InternalGoal* goal)
{
    return goal->mGoal->canContinueToUse();
}

void GoalSelector::tick()
{
    if (mTickCount++ % mNewGoalRate == 0)
    {
        PIXBeginNamedEvent(0.0f, "Updating goals");
        std::vector<InternalGoal*>::iterator available;
        for (available = mAvailableGoals.begin(); available != mAvailableGoals.end(); ++available)
        {
            InternalGoal* goal = *available;
            if (goal->mIsRunning)
            {
                if (!canUseInSystem(goal) || !canContinueToUse(goal))
                {
                    goal->mIsRunning = false;
                    goal->mGoal->stop();
                    mGoals.erase(std::find(mGoals.begin(), mGoals.end(), goal));
                }
            }
            else if (canUseInSystem(goal) && goal->mGoal->canUse())
            {
                goal->mIsRunning = true;
                goal->mGoal->start();
                mGoals.push_back(goal);
            }
        }
        PIXEndNamedEvent();
    }
    else
    {
        PIXBeginNamedEvent(0.0f, "Checking current goals");
        for (std::vector<InternalGoal*>::iterator running = mGoals.begin(); running != mGoals.end();)
        {
            InternalGoal* goal = *running;
            if (!goal->mGoal->canContinueToUse())
            {
                goal->mIsRunning = false;
                goal->mGoal->stop();
                running = mGoals.erase(running);
            }
            else
            {
                ++running;
            }
        }
        PIXEndNamedEvent();
    }
    PIXBeginNamedEvent(0.0f, "Tick using goals");
    if (!mGoals.empty())
    {
        for (unsigned int i = 0; i < mGoals.size(); i++)
        {
            mGoals.at(i)->mGoal->tick();
        }
    }
    PIXEndNamedEvent();
}

bool GoalSelector::isUsingGoal(Goal* goal)
{
    if (goal == 0)
    {
        return false;
    }
    for (unsigned int i = 0; i < mGoals.size(); ++i)
    {
        if (mGoals[i]->mGoal == goal)
        {
            return true;
        }
    }
    return false;
}

void GoalSelector::disableControlFlag(int controlFlag)
{
    mDisabledControlFlags |= controlFlag;
}

void GoalSelector::enableControlFlag(int controlFlag)
{
    mDisabledControlFlags &= ~controlFlag;
}

void GoalSelector::setControlFlag(int controlFlag, bool enabled)
{
    if (enabled)
    {
        enableControlFlag(controlFlag);
    }
    else
    {
        disableControlFlag(controlFlag);
    }
}

void GoalSelector::setLevel(Level* level)
{
    std::vector<InternalGoal*>::iterator it;
    for (it = mAvailableGoals.begin(); it != mAvailableGoals.end(); it++)
    {
        InternalGoal* goal = *it;
        goal->mGoal->setLevel(level);
    }
}
