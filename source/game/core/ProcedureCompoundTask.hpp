#ifndef GAME_CORE_PROCEDURECOMPOUNDTASK_HPP
#define GAME_CORE_PROCEDURECOMPOUNDTASK_HPP

#include <vector>

#include "game/item/not_null_ptr.hpp"
#include "game/minigame/TutorialTask.hpp"

class BlockPos;
class ItemInstance;
class Level;

class ProcedureCompoundTask : public TutorialTask
{
public:
    ProcedureCompoundTask(Tutorial* tutorial) : TutorialTask(tutorial, -1, false, NULL, false, true, false), tasks() {}

    virtual ~ProcedureCompoundTask();

    void AddTask(TutorialTask* task);

    virtual int getDescriptionId();
    virtual int getPromptId();
    virtual bool isCompleted();
    virtual void onCrafted(not_null_ptr<ItemInstance> item);
    virtual void handleUIInput(int input);
    virtual void setAsCurrentTask(bool currentTask);
    virtual bool ShowMinimumTime();
    virtual bool hasBeenActivated();
    virtual void setShownForMinimumTime();
    virtual bool AllowFade();
    virtual void useItemOn(Level* level, not_null_ptr<ItemInstance> item, const BlockPos& pos, bool cancelled);
    virtual void useItem(not_null_ptr<ItemInstance> item, bool cancelled);
    virtual void onTake(not_null_ptr<ItemInstance> item, unsigned int itemCount, unsigned int exactCount);
    virtual void onStateChange(eTutorial_State state);

private:
    std::vector<TutorialTask*> tasks;
};

#endif
