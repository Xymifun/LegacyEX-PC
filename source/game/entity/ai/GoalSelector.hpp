#ifndef GAME_ENTITY_AI_GOALSELECTOR_HPP
#define GAME_ENTITY_AI_GOALSELECTOR_HPP

#include <vector>

class Goal;
class Level;

class GoalSelector
{
public:
    class InternalGoal
    {
    public:
        Goal* mGoal;
        int mPriority;
        bool mIsRunning;
        bool mOwnsGoal;

        InternalGoal(int priority, Goal* goal, bool ownsGoal);
        ~InternalGoal();
    };

    std::vector<InternalGoal*> mAvailableGoals;
    std::vector<InternalGoal*> mGoals;
    int mTickCount;
    int mNewGoalRate;
    int mDisabledControlFlags;

    GoalSelector();
    ~GoalSelector();
    void addGoal(int priority, Goal* goal, bool ownsGoal);
    void removeGoal(Goal* goal);
    bool isControlFlagDisabled(int controlFlag);
    bool canCoExist(InternalGoal* goal, InternalGoal* otherGoal);
    bool canUseInSystem(InternalGoal* goal);
    bool canContinueToUse(InternalGoal* goal);
    void tick();
    bool isUsingGoal(Goal* goal);
    void disableControlFlag(int controlFlag);
    void enableControlFlag(int controlFlag);
    void setControlFlag(int controlFlag, bool enabled);
    void setLevel(Level* level);
    std::vector<InternalGoal*>* getRunningGoals();
    void setNewGoalRate(int rate);
    int getDisabledControlFlags();
};

#endif
