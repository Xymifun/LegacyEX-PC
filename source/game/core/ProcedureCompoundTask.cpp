#include "game/core/ProcedureCompoundTask.hpp"

#include "game/item/not_null_ptr.hpp"
#include "game/minigame/TutorialTask.hpp"
#include "game/world/block/BlockPos.hpp"

ProcedureCompoundTask::~ProcedureCompoundTask()
{
    for (std::vector<TutorialTask*>::iterator it = tasks.begin(); it < tasks.end(); ++it)
    {
        if (*it != NULL)
        {
            delete *it;
        }
    }
}

void ProcedureCompoundTask::AddTask(TutorialTask* task)
{
    if (task != NULL)
    {
        tasks.push_back(task);
    }
}

int ProcedureCompoundTask::getDescriptionId()
{
    if (completed)
    {
        return -1;
    }

    int result = -1;

    for (std::vector<TutorialTask*>::iterator end = tasks.end(), it = tasks.begin(); it < end; ++it)
    {
        TutorialTask* task = *it;
        if (!task->isCompleted())
        {
            task->setAsCurrentTask(true);
            result = task->getDescriptionId();
            break;
        }
        if (task->getCompletionAction() == eTutorial_CompletionAction_CompleteState)
        {
            completed = true;
            break;
        }
    }

    return result;
}

int ProcedureCompoundTask::getPromptId()
{
    if (completed)
    {
        return -1;
    }

    int result = -1;

    for (std::vector<TutorialTask*>::iterator end = tasks.end(), it = tasks.begin(); it < end; ++it)
    {
        TutorialTask* task = *it;
        if (!task->isCompleted())
        {
            result = task->getPromptId();
            break;
        }
    }

    return result;
}

bool ProcedureCompoundTask::isCompleted()
{
    bool result = true;
    bool pending = true;

    for (std::vector<TutorialTask*>::iterator end = tasks.end(), it = tasks.begin(); it < end; ++it)
    {
        TutorialTask* task = *it;
        if (result)
        {
            if (pending)
            {
                if (task->isCompleted())
                {
                    if (task->getCompletionAction() == eTutorial_CompletionAction_CompleteState)
                    {
                        result = true;
                        break;
                    }
                }
                else
                {
                    task->setAsCurrentTask(true);
                    result = false;
                    pending = false;
                }
            }
        }
        else
        {
            task->setAsCurrentTask(false);
        }
    }

    if (result)
    {
        std::vector<TutorialTask*>::iterator it = tasks.begin();
        std::vector<TutorialTask*>::iterator end = tasks.end();
        for (; it < end; ++it)
        {
            (*it)->enableConstraints(false, false);
        }
    }

    completed = result;
    return result;
}

void ProcedureCompoundTask::onCrafted(not_null_ptr<ItemInstance> item)
{
    for (std::vector<TutorialTask*>::iterator end = tasks.end(), it = tasks.begin(); it < end; ++it)
    {
        TutorialTask* task = *it;
        task->onCrafted(item);
    }
}

void ProcedureCompoundTask::handleUIInput(int input)
{
    for (std::vector<TutorialTask*>::iterator end = tasks.end(), it = tasks.begin(); it < end; ++it)
    {
        (*it)->handleUIInput(input);
    }
}

void ProcedureCompoundTask::setAsCurrentTask(bool currentTask)
{
    bool current = true;
    for (std::vector<TutorialTask*>::iterator end = tasks.end(), it = tasks.begin(); it < end; ++it)
    {
        TutorialTask* task = *it;
        if (current)
        {
            if (!task->isCompleted())
            {
                task->setAsCurrentTask(true);
                current = false;
            }
        }
        else
        {
            task->setAsCurrentTask(false);
        }
    }
}

bool ProcedureCompoundTask::ShowMinimumTime()
{
    if (completed)
    {
        return false;
    }

    bool result = false;

    for (std::vector<TutorialTask*>::iterator end = tasks.end(), it = tasks.begin(); it < end; ++it)
    {
        TutorialTask* task = *it;
        if (!task->isCompleted())
        {
            result = task->ShowMinimumTime();
            break;
        }
    }

    return result;
}

bool ProcedureCompoundTask::hasBeenActivated()
{
    if (completed)
    {
        return true;
    }

    bool result = false;

    for (std::vector<TutorialTask*>::iterator end = tasks.end(), it = tasks.begin(); it < end; ++it)
    {
        TutorialTask* task = *it;
        if (!task->isCompleted())
        {
            result = task->hasBeenActivated();
            break;
        }
    }

    return result;
}

void ProcedureCompoundTask::setShownForMinimumTime()
{
    for (std::vector<TutorialTask*>::iterator end = tasks.end(), it = tasks.begin(); it < end; ++it)
    {
        TutorialTask* task = *it;
        if (!task->isCompleted())
        {
            task->setShownForMinimumTime();
            return;
        }
    }
}

bool ProcedureCompoundTask::AllowFade()
{
    if (completed)
    {
        return true;
    }

    bool result = true;

    for (std::vector<TutorialTask*>::iterator end = tasks.end(), it = tasks.begin(); it < end; ++it)
    {
        TutorialTask* task = *it;
        if (!task->isCompleted())
        {
            result = task->AllowFade();
            break;
        }
    }

    return result;
}

void ProcedureCompoundTask::useItemOn(Level* level, not_null_ptr<ItemInstance> item, const BlockPos& pos,
                                      bool cancelled)
{
    for (std::vector<TutorialTask*>::iterator end = tasks.end(), it = tasks.begin(); it < end; ++it)
    {
        TutorialTask* task = *it;
        task->useItemOn(level, item, pos, cancelled);
    }
}

void ProcedureCompoundTask::useItem(not_null_ptr<ItemInstance> item, bool cancelled)
{
    for (std::vector<TutorialTask*>::iterator end = tasks.end(), it = tasks.begin(); it < end; ++it)
    {
        TutorialTask* task = *it;
        task->useItem(item, cancelled);
    }
}

void ProcedureCompoundTask::onTake(not_null_ptr<ItemInstance> item, unsigned int itemCount, unsigned int exactCount)
{
    for (std::vector<TutorialTask*>::iterator end = tasks.end(), it = tasks.begin(); it < end; ++it)
    {
        TutorialTask* task = *it;
        task->onTake(item, itemCount, exactCount);
    }
}

void ProcedureCompoundTask::onStateChange(eTutorial_State state)
{
    for (std::vector<TutorialTask*>::iterator end = tasks.end(), it = tasks.begin(); it < end; ++it)
    {
        (*it)->onStateChange(state);
    }
}
